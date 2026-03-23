/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "task_manager.hpp"

#include <cpu_io.h>
#include <etl/vector.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <new>

#include "basic_info.hpp"
#include "fifo_scheduler.hpp"
#include "kernel_config.hpp"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "rr_scheduler.hpp"
#include "sk_stdlib.h"
#include "task_messages.hpp"
#include "virtual_memory.hpp"

namespace {

auto IdleThread(void*) -> void {
  while (true) {
    per_cpu::GetCurrentCore().sched_data->idle_time++;
    cpu_io::Pause();
  }
}

/// 内核启动时的静态 idle 任务资源（每核心一个）
struct StaticIdleTask {
  TaskControlBlock tcb;
  TaskAuxData aux;
  alignas(cpu_io::virtual_memory::kPageSize)
      uint8_t stack[TaskControlBlock::kDefaultKernelStackSize];
};

std::array<StaticIdleTask, SIMPLEKERNEL_MAX_CORE_COUNT> idle_tasks{};

}  // namespace

auto TaskManager::InitCurrentCore(bool is_primary) -> void {
  auto core_id = cpu_io::GetCurrentCoreId();
  auto& cpu_sched = cpu_schedulers_[core_id];

  LockGuard lock_guard{cpu_sched.lock};

  if (!cpu_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]) {
    cpu_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kRealTime)] =
        kstd::make_unique<FifoScheduler>();
    cpu_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)] =
        kstd::make_unique<RoundRobinScheduler>();
  }

  auto& cpu_data = per_cpu::GetCurrentCore();
  cpu_data.sched_data = &cpu_sched;

  if (is_primary) {
    auto init_task_ptr =
        kstd::make_unique<TaskControlBlock>("init", 10, nullptr, nullptr);
    auto* init_task = init_task_ptr.get();
    init_task->pid = kInitPid;
    init_task->aux->tgid = kInitPid;
    init_task->policy = SchedPolicy::kNormal;
    init_task->fsm.Receive(MsgSchedule{});
    init_task->fsm.Receive(MsgSchedule{});

    {
      LockGuard<SpinLock> table_guard(task_table_lock_);
      task_table_[init_task->pid] = std::move(init_task_ptr);
    }

    cpu_data.running_task = init_task;
  } else {
    auto boot_task_ptr = kstd::make_unique<TaskControlBlock>(
        "boot",
        std::numeric_limits<
            decltype(TaskControlBlock::SchedInfo::priority)>::max(),
        nullptr, nullptr);
    auto* boot_task = boot_task_ptr.release();
    boot_task->fsm.Receive(MsgSchedule{});
    boot_task->fsm.Receive(MsgSchedule{});
    boot_task->policy = SchedPolicy::kIdle;
    cpu_data.running_task = boot_task;
  }

  auto& idle_res = idle_tasks[core_id];
  auto* idle_task = &idle_res.tcb;

  idle_task->name = "idle";
  idle_task->pid = AllocatePid();
  idle_task->policy = SchedPolicy::kIdle;
  idle_task->owns_resources = false;

  idle_task->aux = &idle_res.aux;
  idle_task->kernel_stack = idle_res.stack;

  idle_task->sched_info.priority = std::numeric_limits<
      decltype(TaskControlBlock::SchedInfo::priority)>::max();
  idle_task->sched_info.base_priority = idle_task->sched_info.priority;

  idle_task->trap_context_ptr = reinterpret_cast<cpu_io::TrapContext*>(
      idle_res.stack + TaskControlBlock::kDefaultKernelStackSize -
      sizeof(cpu_io::TrapContext));

  auto stack_top = reinterpret_cast<uint64_t>(idle_res.stack) +
                   TaskControlBlock::kDefaultKernelStackSize;
  InitTaskContext(&idle_task->task_context, IdleThread, nullptr, stack_top);

  idle_task->fsm.Start();
  idle_task->fsm.Receive(MsgSchedule{});

  cpu_data.idle_task = idle_task;
}

auto TaskManager::AddTask(etl::unique_ptr<TaskControlBlock> task) -> void {
  assert(task.get() != nullptr && "AddTask: task must not be null");
  assert(task->GetStatus() == TaskStatus::kUnInit &&
         "AddTask: task status must be kUnInit");
  // 分配 PID
  if (task->pid == 0) {
    task->pid = AllocatePid();
  }

  // 如果 tgid 未设置，则将其设为自己的 pid (单线程进程或线程组的主线程)
  if (task->aux->tgid == 0) {
    task->aux->tgid = task->pid;
  }

  auto* task_ptr = task.get();
  Pid pid = task_ptr->pid;

  // 确定目标核心
  auto target_core = cpu_io::GetCurrentCoreId();
  if (task_ptr->aux->cpu_affinity.value() != UINT64_MAX) {
    // 寻找第一个允许的核心
    for (size_t core_id = 0; core_id < SIMPLEKERNEL_MAX_CORE_COUNT; ++core_id) {
      if (task_ptr->aux->cpu_affinity.value() & (1UL << core_id)) {
        target_core = core_id;
        break;
      }
    }
  }

  // 设置任务状态为 kReady（task 尚未对其他核心可见，无需锁）
  // Transition: kUnInit -> kReady
  task_ptr->fsm.Receive(MsgSchedule{});

  auto& cpu_sched = cpu_schedulers_[target_core];

  // 锁序统一为 sched_lock → task_table_lock（与 Exit/Wait 一致，防止死锁）
  {
    LockGuard<SpinLock> sched_guard(cpu_sched.lock);
    {
      LockGuard<SpinLock> table_guard(task_table_lock_);
      if (task_table_.full()) {
        klog::Err("AddTask: task_table_ full, cannot add task (pid={})", pid);
        return;
      }
      task_table_[pid] = std::move(task);
    }

    assert(task_ptr->policy < SchedPolicy::kPolicyCount &&
           "AddTask: invalid scheduling policy");
    if (cpu_sched.schedulers[static_cast<uint8_t>(task_ptr->policy)]) {
      cpu_sched.schedulers[static_cast<uint8_t>(task_ptr->policy)]->Enqueue(
          task_ptr);
    }
  }
}

auto TaskManager::AllocatePid() -> size_t {
  /// @note 当前 PID 分配器为简单的原子自增，存在以下限制：
  ///   1. 不支持 PID 回收与重用（已退出的任务的 PID 不会被回收）
  ///   2. 不检测溢出（size_t 耗尽后回绕为 0，可能与现有 PID 冲突）
  ///   3. 不保证全局唯一性（依赖 size_t 足够大 + 系统生命周期内不会耗尽）
  /// 对于教学内核而言，size_t 的范围（2^64）在实际使用中不会溢出。
  /// 生产级实现应使用位图或 ID 分配器（如 Linux 的 IDR/IDA）。
  return pid_allocator_.fetch_add(1);
}

auto TaskManager::FindTask(Pid pid) -> TaskControlBlock* {
  LockGuard lock_guard{task_table_lock_};
  auto it = task_table_.find(pid);
  return (it != task_table_.end()) ? it->second.get() : nullptr;
}

auto TaskManager::ReapTask(TaskControlBlock* task) -> void {
  if (!task) {
    return;
  }

  // 确保任务处于僵尸或退出状态
  if (task->GetStatus() != TaskStatus::kZombie &&
      task->GetStatus() != TaskStatus::kExited) {
    klog::Warn("ReapTask: Task {} is not in zombie/exited state", task->pid);
    return;
  }

  // Capture pid before erase (unique_ptr deletes on erase)
  Pid pid = task->pid;

  // 从全局任务表中移除 (unique_ptr auto-deletes TCB)
  {
    LockGuard lock_guard{task_table_lock_};
    task_table_.erase(pid);
  }

  klog::Debug("ReapTask: Task {} resources freed", pid);
}

auto TaskManager::ReparentChildren(TaskControlBlock* parent) -> void {
  if (!parent) {
    return;
  }

  LockGuard lock_guard{task_table_lock_};

  // 遍历所有任务，找到父进程是当前任务的子进程
  for (auto& [pid, task] : task_table_) {
    if (task && task->aux->parent_pid == parent->pid) {
      // 将子进程过继给 init 进程
      task->aux->parent_pid = kInitPid;
      klog::Debug("ReparentChildren: Task {} reparented to init (PID {})",
                  task->pid, kInitPid);
    }
  }
}

auto TaskManager::GetThreadGroup(Pid tgid)
    -> etl::vector<TaskControlBlock*, kernel::config::kMaxReadyTasks> {
  etl::vector<TaskControlBlock*, kernel::config::kMaxReadyTasks> result;

  LockGuard lock_guard(task_table_lock_);

  // 遍历任务表，找到所有 tgid 匹配的线程
  for (auto& [pid, task] : task_table_) {
    if (task && task->aux->tgid == tgid) {
      result.push_back(task.get());
    }
  }

  return result;
}

auto TaskManager::SignalThreadGroup(Pid tgid, int signal) -> void {
  auto threads = GetThreadGroup(tgid);
  for (auto* thread : threads) {
    (void)SendSignal(thread->pid, signal);
  }
  klog::Debug("SignalThreadGroup: tgid={}, signal={}, count={}", tgid, signal,
              threads.size());
}

TaskManager::~TaskManager() {
  // unique_ptr in cpu_schedulers_.schedulers[] auto-deletes on destruction
}

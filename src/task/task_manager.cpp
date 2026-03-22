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
#include "idle_scheduler.hpp"
#include "kernel_config.hpp"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "rr_scheduler.hpp"
#include "sk_stdlib.h"
#include "task_messages.hpp"
#include "virtual_memory.hpp"

namespace {

/// idle 线程入口函数
auto IdleThread(void*) -> void {
  while (true) {
    per_cpu::GetCurrentCore().sched_data->idle_time++;
    cpu_io::Pause();
  }
}

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
    cpu_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kIdle)] =
        kstd::make_unique<IdleScheduler>();
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

  auto idle_task_ptr = kstd::make_unique<TaskControlBlock>(
      "idle",
      std::numeric_limits<
          decltype(TaskControlBlock::SchedInfo::priority)>::max(),
      IdleThread, nullptr);
  auto* idle_task = idle_task_ptr.release();
  idle_task->fsm.Receive(MsgSchedule{});
  idle_task->policy = SchedPolicy::kIdle;

  if (cpu_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kIdle)]) {
    cpu_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kIdle)]->Enqueue(
        idle_task);
  }

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

auto TaskManager::Balance() -> void {
  auto current_core = cpu_io::GetCurrentCoreId();
  auto& current_sched = cpu_schedulers_[current_core];

  // 获取当前核心 kNormal 队列长度（无锁快速检查）
  size_t current_load = 0;
  if (current_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]) {
    current_load =
        current_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]
            ->GetQueueSize();
  }

  // 寻找负载最高的核心
  size_t max_load = 0;
  size_t max_core = current_core;

  for (size_t core_id = 0; core_id < SIMPLEKERNEL_MAX_CORE_COUNT; ++core_id) {
    if (core_id == current_core) {
      continue;
    }
    auto& other_sched = cpu_schedulers_[core_id];
    if (other_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]) {
      size_t load =
          other_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]
              ->GetQueueSize();
      if (load > max_load) {
        max_load = load;
        max_core = core_id;
      }
    }
  }

  // 仅当差值 > 1 时才窃取（避免 ping-pong）
  if (max_core == current_core || max_load <= current_load + 1) {
    return;
  }

  // 按核心 ID 顺序获取锁，防止死锁
  auto& source_sched = cpu_schedulers_[max_core];
  size_t first_core = (current_core < max_core) ? current_core : max_core;
  size_t second_core = (current_core < max_core) ? max_core : current_core;

  LockGuard<SpinLock> lock_first(cpu_schedulers_[first_core].lock);
  LockGuard<SpinLock> lock_second(cpu_schedulers_[second_core].lock);

  // 重新检查（持锁后条件可能已变化）
  auto* source_scheduler =
      source_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)].get();
  auto* dest_scheduler =
      current_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]
          .get();

  if (!source_scheduler || !dest_scheduler) {
    return;
  }

  size_t source_load = source_scheduler->GetQueueSize();
  size_t dest_load = dest_scheduler->GetQueueSize();

  if (source_load <= dest_load + 1) {
    return;
  }

  auto* stolen = source_scheduler->PickNext();
  if (!stolen) {
    return;
  }

  if (stolen->aux && stolen->aux->cpu_affinity.value() != UINT64_MAX &&
      !(stolen->aux->cpu_affinity.value() & (1UL << current_core))) {
    source_scheduler->Enqueue(stolen);
    return;
  }

  dest_scheduler->Enqueue(stolen);
  klog::Debug("Balance: Stole task '{}' (pid={}) from core {} to core {}",
              stolen->name, stolen->pid, max_core, current_core);
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
  /// @todo 实现信号机制后，向线程组中的所有线程发送信号
  klog::Debug("SignalThreadGroup: tgid={}, signal={} (not implemented)", tgid,
              signal);

  // 预期实现：
  // auto threads = GetThreadGroup(tgid);
  // for (auto* thread : threads) {
  //   SendSignal(thread, signal);
  // }
}

TaskManager::~TaskManager() {
  // unique_ptr in cpu_schedulers_.schedulers[] auto-deletes on destruction
}

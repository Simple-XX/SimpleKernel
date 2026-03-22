/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <new>

#include "arch.h"
#include "basic_info.hpp"
#include "fifo_scheduler.hpp"
#include "interrupt_base.h"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "per_cpu.hpp"
#include "sk_stdlib.h"
#include "spinlock.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"
#include "virtual_memory.hpp"

namespace {
/**
 * @brief 纯解锁，不碰中断（对应 Linux finish_task_switch → raw_spin_unlock）
 * @pre  当前 CPU 的 sched_lock 由 Schedule() 持有（lock handoff）
 * @post sched_lock 已释放，中断状态不变
 */
void FinishSwitch() {
  per_cpu::GetCurrentCore().sched_data->lock.UnLock().or_else([](auto&& err) {
    klog::Err("FinishSwitch: Failed to release sched_lock: {}", err.message());
    while (true) {
      cpu_io::Pause();
    }
    return Expected<void>{};
  });
}

void HandleDeferredCleanup() {
  auto* sched_data = per_cpu::GetCurrentCore().sched_data;
  auto* prev = sched_data->prev_task;
  sched_data->prev_task = nullptr;

  if (prev != nullptr && prev->GetStatus() == TaskStatus::kZombie &&
      prev->aux != nullptr && prev->aux->parent_pid != 0) {
    /// @todo 信号子系统就绪后，向 parent_pid 投递 SIGCHLD
    TaskManagerSingleton::instance().Wakeup(
        ResourceId(ResourceType::kChildExit, prev->aux->parent_pid));
  }
}
}  // namespace

extern "C" [[noreturn]] void kernel_thread_bootstrap(void (*entry)(void*),
                                                     void* arg) {
  FinishSwitch();
  HandleDeferredCleanup();
  cpu_io::EnableInterrupt();
  entry(arg);
  assert(false && "kernel thread returned without calling sys_exit()");
  while (true) {
    cpu_io::Pause();
  }
}

auto TaskManager::Schedule() -> void {
  auto& cpu_sched = GetCurrentCpuSched();

  // saved_intr 在任务内核栈上，switch_to 时随栈帧保存/恢复，
  // 每个任务恢复自己的调用方中断状态，避免跨 switch_to 的 lock handoff 污染。
  auto saved_intr = cpu_io::GetInterruptStatus();
  cpu_io::DisableInterrupt();

  cpu_sched.lock.Lock().or_else([](auto&& err) {
    klog::Err("Schedule: Failed to acquire lock: {}", err.message());
    while (true) {
      cpu_io::Pause();
    }
    return Expected<void>{};
  });

  cpu_sched.scheduler_started = true;

  auto* current = GetCurrentTask();
  assert(current != nullptr && "Schedule: No current task to schedule");

  if (current->GetStatus() == TaskStatus::kRunning) {
    current->fsm.Receive(MsgYield{});
    auto* scheduler =
        cpu_sched.schedulers[static_cast<uint8_t>(current->policy)].get();

    if (scheduler) {
      scheduler->OnPreempted(current);
      if (scheduler->OnTimeSliceExpired(current)) {
        scheduler->Enqueue(current);
      }
    }
  }

  TaskControlBlock* next = nullptr;
  for (auto& scheduler : cpu_sched.schedulers) {
    if (!scheduler || scheduler->IsEmpty()) {
      continue;
    }
    next = scheduler->PickNext();
    if (!next) {
      continue;
    }
    break;
  }

  assert(next != nullptr &&
         "Schedule: no runnable task (idle task missing from scheduler)");
  assert(next->GetStatus() == TaskStatus::kReady &&
         "Schedule: picked task must be in kReady state");

  next->fsm.Receive(MsgSchedule{});
  next->sched_info.time_slice_remaining = next->sched_info.time_slice_default;
  next->sched_info.context_switches++;
  cpu_sched.total_schedules++;

  auto* scheduler =
      cpu_sched.schedulers[static_cast<uint8_t>(next->policy)].get();
  if (scheduler) {
    scheduler->OnScheduled(next);
  }

  cpu_sched.prev_task = current;
  per_cpu::GetCurrentCore().running_task = next;

  if (current != next) {
    switch_to(&current->task_context, &next->task_context);
  }

  FinishSwitch();

  if (saved_intr) {
    cpu_io::EnableInterrupt();
  }

  HandleDeferredCleanup();
}

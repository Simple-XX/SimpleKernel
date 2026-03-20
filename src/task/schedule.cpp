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

extern "C" [[noreturn]] void kernel_thread_bootstrap(void (*entry)(void*),
                                                     void* arg) {
  cpu_io::EnableInterrupt();
  entry(arg);
  assert(false && "kernel thread returned without calling sys_exit()");
  while (true) {
    cpu_io::Pause();
  }
}

auto TaskManager::Schedule() -> void {
  auto& cpu_sched = GetCurrentCpuSched();
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

  // 处理当前任务状态
  if (current->GetStatus() == TaskStatus::kRunning) {
    // 将当前任务标记为就绪并重新入队（如果它还能运行）
    current->fsm.Receive(MsgYield{});
    auto* scheduler =
        cpu_sched.schedulers[static_cast<uint8_t>(current->policy)].get();

    if (scheduler) {
      scheduler->OnPreempted(current);
      // 调度器决定如何处理被抢占的任务
      // 大多数情况下需要重新入队，除非是特殊策略
      if (scheduler->OnTimeSliceExpired(current)) {
        scheduler->Enqueue(current);
      }
    }
  }

  // 选择下一个任务 (按策略优先级: RealTime > Normal > Idle)
  TaskControlBlock* next = nullptr;
  for (auto& scheduler : cpu_sched.schedulers) {
    if (scheduler && !scheduler->IsEmpty()) {
      next = scheduler->PickNext();
      if (next) {
        break;
      }
    }
  }

  // 如果没有任务可运行
  if (!next) {
    // 如果当前任务仍然可以运行，继续运行它
    if (current->GetStatus() == TaskStatus::kReady) {
      next = current;
    } else {
      // 否则统计空闲时间并返回
      cpu_sched.idle_time++;
      cpu_sched.lock.UnLock().or_else([](auto&& err) {
        klog::Err("Schedule: Failed to release lock: {}", err.message());
        while (true) {
          cpu_io::Pause();
        }
        return Expected<void>{};
      });
      return;
    }
  }

  // 切换到下一个任务
  assert(next != nullptr && "Schedule: next task must not be null");
  assert((next->GetStatus() == TaskStatus::kReady ||
          next->policy == SchedPolicy::kIdle) &&
         "Schedule: next task must be kReady or kIdle policy");

  next->fsm.Receive(MsgSchedule{});
  // 重置时间片（对于 RR 和 FIFO 有效，CFS 使用 vruntime 不依赖此字段）
  next->sched_info.time_slice_remaining = next->sched_info.time_slice_default;
  next->sched_info.context_switches++;
  cpu_sched.total_schedules++;

  // 调用调度器钩子
  auto* scheduler =
      cpu_sched.schedulers[static_cast<uint8_t>(next->policy)].get();
  if (scheduler) {
    scheduler->OnScheduled(next);
  }

  // 更新 per-CPU running_task
  per_cpu::GetCurrentCore().running_task = next;

  cpu_sched.lock.UnLock().or_else([](auto&& err) {
    klog::Err("Schedule: Failed to release lock: {}", err.message());
    while (true) {
      cpu_io::Pause();
    }
    return Expected<void>{};
  });

  // 上下文切换
  if (current != next) {
    switch_to(&current->task_context, &next->task_context);
  }
}

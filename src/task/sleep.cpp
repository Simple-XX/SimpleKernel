/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>

#include "kernel_log.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

/// 每秒的毫秒数
static constexpr uint64_t kMillisecondsPerSecond = 1000;

auto TaskManager::Sleep(uint64_t ms) -> void {
  auto& cpu_sched = GetCurrentCpuSched();

  auto* current = GetCurrentTask();
  assert(current != nullptr && "Sleep: No current task to sleep");
  assert(current->GetStatus() == TaskStatus::kRunning &&
         "Sleep: current task status must be kRunning");

  // 如果睡眠时间为 0，仅让出 CPU（相当于 yield）
  if (ms == 0) {
    Schedule();
    return;
  }

  {
    LockGuard<SpinLock> lock_guard(cpu_sched.lock);

    // 计算唤醒时间 (当前 tick + 睡眠时间)
    uint64_t sleep_ticks = (ms * SIMPLEKERNEL_TICK) / kMillisecondsPerSecond;
    current->sched_info.wake_tick = cpu_sched.local_tick + sleep_ticks;

    // Check capacity before transitioning FSM

    // 将任务加入睡眠队列（优先队列会自动按 wake_tick 排序）
    if (cpu_sched.sleeping_tasks.full()) {
      klog::Err("Sleep: sleeping_tasks full, cannot sleep task {}",
                current->pid);
      return;
    }
    current->fsm.Receive(MsgSleep{current->sched_info.wake_tick});
    cpu_sched.sleeping_tasks.push(current);
  }

  // 调度到其他任务
  Schedule();

  // 任务被唤醒后会从这里继续执行
  // 检查是否有待处理的信号（可能是信号导致了提前唤醒）
  (void)CheckPendingSignals();
}

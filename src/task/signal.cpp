/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "signal.hpp"

#include <cassert>

#include "kernel_log.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

auto TaskManager::SendSignal(Pid pid, int signum) -> Expected<void> {
  if (!SignalState::IsValid(signum)) {
    return std::unexpected(Error{ErrorCode::kSignalInvalidNumber});
  }

  // 查找目标任务
  TaskControlBlock* target = nullptr;
  {
    LockGuard<SpinLock> lock_guard(task_table_lock_);
    auto it = task_table_.find(pid);
    if (it == task_table_.end()) {
      return std::unexpected(Error{ErrorCode::kSignalTaskNotFound});
    }
    target = it->second.get();
  }

  if (!target || !target->aux) {
    return std::unexpected(Error{ErrorCode::kSignalTaskNotFound});
  }

  target->aux->signals.SetPending(signum);

  klog::Debug("SendSignal: signal {} ({}) sent to pid={}",
              SignalState::GetSignalName(signum), signum, pid);

  // SIGKILL 和 SIGCONT 需要立即唤醒阻塞/睡眠的任务
  bool needs_wake =
      (signum == signal_number::kSigKill || signum == signal_number::kSigCont);

  // 如果信号未被屏蔽，也需要唤醒
  if (!needs_wake &&
      !(target->aux->signals.blocked.load(std::memory_order_acquire) &
        (1U << signum))) {
    auto status = target->GetStatus();
    if (status == TaskStatus::kBlocked || status == TaskStatus::kSleeping) {
      needs_wake = true;
    }
  }

  if (needs_wake) {
    auto status = target->GetStatus();

    if (status == TaskStatus::kBlocked) {
      auto resource_id = target->aux->blocked_on;
      if (static_cast<bool>(resource_id)) {
        Wakeup(resource_id);
        klog::Debug("SendSignal: woke blocked task pid={} for signal {}", pid,
                    SignalState::GetSignalName(signum));
      }
    } else if (status == TaskStatus::kSleeping) {
      // Expedite wake: set wake_tick to 0 so next TickUpdate wakes it.
      // Note: single aligned 64-bit store is atomic on 64-bit platforms.
      target->sched_info.wake_tick = 0;
      klog::Debug("SendSignal: expedited sleep wakeup for pid={}", pid);
    }
  }

  return {};
}

auto TaskManager::CheckPendingSignals() -> int {
  auto* current = GetCurrentTask();
  if (!current || !current->aux) {
    return 0;
  }

  auto& signals = current->aux->signals;
  if (!signals.HasDeliverableSignal()) {
    return 0;
  }

  int signum = signals.GetNextDeliverableSignal();
  if (signum == signal_number::kSigNone) {
    return 0;
  }

  signals.ClearPending(signum);

  klog::Debug("CheckPendingSignals: pid={} delivering signal {} ({})",
              current->pid, signum, SignalState::GetSignalName(signum));

  auto& action = signals.actions[signum];

  // SIGKILL: 强制终止，无法捕获
  if (signum == signal_number::kSigKill) {
    klog::Info("CheckPendingSignals: pid={} killed by SIGKILL", current->pid);
    Exit(128 + signum);
    __builtin_unreachable();
  }

  // SIGSTOP: 强制停止，无法捕获
  /// @todo 实现进程停止状态 (需要新的 FSM 状态 kStopped)
  if (signum == signal_number::kSigStop || signum == signal_number::kSigTstp) {
    klog::Info(
        "CheckPendingSignals: pid={} received stop signal {} (not implemented)",
        current->pid, SignalState::GetSignalName(signum));
    return signum;
  }

  // SIGCONT: 继续（如果已停止）
  if (signum == signal_number::kSigCont) {
    klog::Debug("CheckPendingSignals: pid={} received SIGCONT", current->pid);
    return signum;
  }

  if (action.handler == kSigIgn) {
    klog::Debug("CheckPendingSignals: pid={} ignored signal {}", current->pid,
                SignalState::GetSignalName(signum));
    return signum;
  }

  if (action.handler == kSigDfl) {
    char default_action = GetDefaultSignalAction(signum);
    switch (default_action) {
      case 'T':
      case 'C':
        klog::Info(
            "CheckPendingSignals: pid={} terminated by signal {} (default)",
            current->pid, SignalState::GetSignalName(signum));
        Exit(128 + signum);
        __builtin_unreachable();
      case 'I':
        break;
      case 'S':
        klog::Debug("CheckPendingSignals: pid={} stop by {} (not implemented)",
                    current->pid, SignalState::GetSignalName(signum));
        break;
      case 'K':
        break;
      default:
        break;
    }
    return signum;
  }

  /// @todo 实现用户空间信号投递：
  /// 1. 保存当前 trap 上下文
  /// 2. 修改 trap 上下文的 PC 指向 handler
  /// 3. 在用户栈上构造 sigreturn 蹦床
  /// 4. 返回用户空间执行 handler
  /// 5. handler 返回时通过 sigreturn 恢复原始上下文
  klog::Debug(
      "CheckPendingSignals: pid={} user handler for signal {} (delivery "
      "not yet implemented)",
      current->pid, SignalState::GetSignalName(signum));
  return signum;
}

auto TaskManager::SetSignalAction(int signum, const SignalAction& action,
                                  SignalAction* old_action) -> Expected<void> {
  auto* current = GetCurrentTask();
  if (!current || !current->aux) {
    return std::unexpected(Error{ErrorCode::kTaskNoCurrentTask});
  }

  if (!SignalState::IsValid(signum)) {
    return std::unexpected(Error{ErrorCode::kSignalInvalidNumber});
  }

  if (SignalState::IsUncatchable(signum)) {
    return std::unexpected(Error{ErrorCode::kSignalUncatchable});
  }

  auto& signals = current->aux->signals;

  if (old_action) {
    *old_action = signals.actions[signum];
  }

  signals.actions[signum] = action;

  klog::Debug("SetSignalAction: pid={} set handler for signal {} ({})",
              current->pid, signum, SignalState::GetSignalName(signum));

  return {};
}

auto TaskManager::SetSignalMask(int how, uint32_t set, uint32_t* oldset)
    -> Expected<void> {
  auto* current = GetCurrentTask();
  if (!current || !current->aux) {
    return std::unexpected(Error{ErrorCode::kTaskNoCurrentTask});
  }

  auto& signals = current->aux->signals;

  if (oldset) {
    *oldset = signals.blocked.load(std::memory_order_acquire);
  }

  // SIGKILL 和 SIGSTOP 不能被屏蔽
  uint32_t uncatchable_mask =
      (1U << signal_number::kSigKill) | (1U << signal_number::kSigStop);
  set &= ~uncatchable_mask;

  switch (how) {
    case signal_mask_op::kSigBlock:
      signals.blocked.fetch_or(set, std::memory_order_release);
      break;
    case signal_mask_op::kSigUnblock:
      signals.blocked.fetch_and(~set, std::memory_order_release);
      break;
    case signal_mask_op::kSigSetmask:
      signals.blocked.store(set, std::memory_order_release);
      break;
    default:
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
  }

  signals.blocked.fetch_and(~uncatchable_mask, std::memory_order_release);

  klog::Debug("SetSignalMask: pid={} mask={:#x}", current->pid,
              signals.blocked.load(std::memory_order_acquire));

  return {};
}

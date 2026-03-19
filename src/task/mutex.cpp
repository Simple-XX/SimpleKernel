/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "mutex.hpp"

#include "kernel.h"
#include "kernel_log.hpp"
#include "task_manager.hpp"

auto Mutex::Lock() -> Expected<void> {
  auto& tm = TaskManagerSingleton::instance();
  auto* current_task = tm.GetCurrentTask();
  if (current_task == nullptr) {
    klog::Err("Mutex::Lock: Cannot lock mutex '{}' outside task context", name);
    return std::unexpected(Error{ErrorCode::kMutexNoTaskContext});
  }

  Pid current_pid = current_task->pid;

  if (IsLockedByCurrentTask()) {
    klog::Warn("Mutex::Lock: Task {} tried to recursively lock mutex '{}'",
               current_pid, name);
    return std::unexpected(Error{ErrorCode::kMutexRecursiveLock});
  }

  // 快速路径：尝试立即获取
  bool expected = false;
  if (locked_.compare_exchange_strong(expected, true, std::memory_order_acquire,
                                      std::memory_order_relaxed)) {
    owner_.store(current_pid, std::memory_order_release);
    klog::Debug("Mutex::Lock: Task {} acquired mutex '{}'", current_pid, name);
    return {};
  }

  // 慢路径：在调度器锁保护下进行 re-check + block，防止丢失唤醒
  while (true) {
    {
      auto& cpu_sched = tm.GetCurrentCpuSched();
      LockGuard<SpinLock> lock_guard(cpu_sched.lock);

      expected = false;
      if (locked_.compare_exchange_strong(expected, true,
                                          std::memory_order_acquire,
                                          std::memory_order_relaxed)) {
        owner_.store(current_pid, std::memory_order_release);
        klog::Debug("Mutex::Lock: Task {} acquired mutex '{}' (re-check)",
                    current_pid, name);
        return {};
      }

      klog::Debug("Mutex::Lock: Task {} blocking on mutex '{}'", current_pid,
                  name);
      tm.Block(cpu_sched, resource_id_);
    }

    tm.Schedule();

    expected = false;
    if (locked_.compare_exchange_strong(expected, true,
                                        std::memory_order_acquire,
                                        std::memory_order_relaxed)) {
      owner_.store(current_pid, std::memory_order_release);
      klog::Debug("Mutex::Lock: Task {} acquired mutex '{}' (after wake)",
                  current_pid, name);
      return {};
    }
  }
}

auto Mutex::UnLock() -> Expected<void> {
  auto current_task = TaskManagerSingleton::instance().GetCurrentTask();
  if (current_task == nullptr) {
    klog::Err("Mutex::UnLock: Cannot unlock mutex '{}' outside task context",
              name);
    return std::unexpected(Error{ErrorCode::kMutexNoTaskContext});
  }

  Pid current_pid = current_task->pid;

  if (!IsLockedByCurrentTask()) {
    klog::Warn(
        "Mutex::UnLock: Task {} tried to unlock mutex '{}' it doesn't own",
        current_pid, name);
    return std::unexpected(Error{ErrorCode::kMutexNotOwned});
  }

  owner_.store(std::numeric_limits<Pid>::max(), std::memory_order_release);
  locked_.store(false, std::memory_order_release);

  klog::Debug("Mutex::UnLock: Task {} released mutex '{}'", current_pid, name);

  TaskManagerSingleton::instance().Wakeup(resource_id_);

  return {};
}

auto Mutex::TryLock() -> Expected<void> {
  auto current_task = TaskManagerSingleton::instance().GetCurrentTask();
  if (current_task == nullptr) {
    klog::Err("Mutex::TryLock: Cannot trylock mutex '{}' outside task context",
              name);
    return std::unexpected(Error{ErrorCode::kMutexNoTaskContext});
  }

  Pid current_pid = current_task->pid;

  if (IsLockedByCurrentTask()) {
    klog::Debug(
        "Mutex::TryLock: Task {} tried to recursively trylock mutex '{}'",
        current_pid, name);
    return std::unexpected(Error{ErrorCode::kMutexRecursiveLock});
  }

  bool expected = false;
  if (locked_.compare_exchange_strong(expected, true, std::memory_order_acquire,
                                      std::memory_order_relaxed)) {
    owner_.store(current_pid, std::memory_order_release);
    klog::Debug("Mutex::TryLock: Task {} acquired mutex '{}'", current_pid,
                name);
    return {};
  }

  klog::Debug("Mutex::TryLock: Task {} failed to acquire mutex '{}'",
              current_pid, name);
  return std::unexpected(Error{ErrorCode::kMutexNotLocked});
}

auto Mutex::IsLockedByCurrentTask() const -> bool {
  auto current_task = TaskManagerSingleton::instance().GetCurrentTask();
  if (current_task == nullptr) {
    return false;
  }

  return locked_.load(std::memory_order_acquire) &&
         owner_.load(std::memory_order_acquire) == current_task->pid;
}

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>

#include "expected.hpp"
#include "kernel_log.hpp"
#include "resource_id.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

auto TaskManager::Wait(Pid pid, int* status, bool no_hang, bool untraced)
    -> Expected<Pid> {
  auto* current = GetCurrentTask();
  assert(current != nullptr && "Wait: No current task");
  assert(current->GetStatus() == TaskStatus::kRunning &&
         "Wait: current task status must be kRunning");

  while (true) {
    TaskControlBlock* target = nullptr;
    bool did_block = false;
    bool has_matching_child = false;

    {
      // 锁顺序: cpu_sched.lock → task_table_lock_（与 Exit 一致）
      auto& cpu_sched = GetCurrentCpuSched();
      LockGuard<SpinLock> sched_guard(cpu_sched.lock);
      LockGuard<SpinLock> table_guard(task_table_lock_);

      for (auto& [task_pid, task] : task_table_) {
        bool is_child = (task->aux->parent_pid == current->pid);

        bool pid_match = false;
        if (pid == static_cast<Pid>(-1)) {
          pid_match = is_child;
        } else if (pid == 0) {
          pid_match = is_child && (task->aux->pgid == current->aux->pgid);
        } else if (pid > 0) {
          pid_match = is_child && (task->pid == pid);
        } else {
          pid_match = is_child && (task->aux->pgid == static_cast<Pid>(-pid));
        }

        if (!pid_match) {
          continue;
        }

        has_matching_child = true;

        if (task->GetStatus() == TaskStatus::kZombie ||
            task->GetStatus() == TaskStatus::kExited) {
          target = task.get();
          break;
        }

        if (untraced && task->GetStatus() == TaskStatus::kBlocked) {
          if (status) {
            *status = 0;
          }
          return task->pid;
        }
      }

      // 没有匹配的子进程，返回错误（类似 POSIX ECHILD）
      if (!has_matching_child) {
        return std::unexpected(Error{ErrorCode::kTaskNoChildFound});
      }

      // 原子性：在持有两把锁的情况下检查并阻塞，防止丢失唤醒
      if (!target && !no_hang) {
        auto wait_resource_id =
            ResourceId(ResourceType::kChildExit, current->pid);
        Block(cpu_sched, wait_resource_id);
        did_block = true;
      }
    }

    if (target) {
      assert((target->GetStatus() == TaskStatus::kZombie ||
              target->GetStatus() == TaskStatus::kExited) &&
             "Wait: target task must be kZombie or kExited");

      Pid result_pid = target->pid;

      if (status) {
        *status = target->aux->exit_code;
      }

      {
        LockGuard<SpinLock> lock_guard(task_table_lock_);
        task_table_.erase(target->pid);
      }

      klog::Debug("Wait: pid={} reaped child={}", current->pid, result_pid);
      return result_pid;
    }

    if (no_hang) {
      return 0;
    }

    Schedule();
  }
}

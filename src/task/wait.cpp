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

    {
      LockGuard lock_guard(task_table_lock_);

      // 遍历任务表寻找符合条件的子进程
      for (auto& [task_pid, task] : task_table_) {
        // 检查是否是当前进程的子进程
        bool is_child = (task->aux->parent_pid == current->pid);

        // 检查 PID 匹配条件
        bool pid_match = false;
        if (pid == static_cast<Pid>(-1)) {
          // 等待任意子进程
          pid_match = is_child;
        } else if (pid == 0) {
          // 等待同进程组的任意子进程
          pid_match = is_child && (task->aux->pgid == current->aux->pgid);
        } else if (pid > 0) {
          // 等待指定 PID 的子进程
          pid_match = is_child && (task->pid == pid);
        } else {
          // pid < -1: 等待进程组 ID 为 |pid| 的任意子进程
          pid_match = is_child && (task->aux->pgid == static_cast<Pid>(-pid));
        }

        if (!pid_match) {
          continue;
        }

        // 检查任务状态
        if (task->GetStatus() == TaskStatus::kZombie ||
            task->GetStatus() == TaskStatus::kExited) {
          target = task.get();
          break;
        }

        // untraced: 报告已停止的子进程
        if (untraced && task->GetStatus() == TaskStatus::kBlocked) {
          if (status) {
            // 表示停止状态
            *status = 0;
          }
          return task->pid;
        }
      }
    }

    // 找到了退出的子进程
    if (target) {
      assert((target->GetStatus() == TaskStatus::kZombie ||
              target->GetStatus() == TaskStatus::kExited) &&
             "Wait: target task must be kZombie or kExited");
      assert(target->aux->parent_pid == current->pid &&
             "Wait: target parent_pid must match current pid");

      Pid result_pid = target->pid;

      // 返回退出状态
      if (status) {
        *status = target->aux->exit_code;
      }

      // 清理僵尸进程
      {
        LockGuard lock_guard(task_table_lock_);
        auto it = task_table_.find(target->pid);
        assert(it != task_table_.end() &&
               "Wait: target must exist in task_table");
        task_table_.erase(it->first);
      }

      klog::Debug("Wait: pid={} reaped child={}", current->pid, result_pid);
      return result_pid;
    }

    // 如果设置了 no_hang，立即返回
    if (no_hang) {
      return 0;
    }

    // 没有找到符合条件的子进程，阻塞等待
    // 使用 ChildExit 类型的资源 ID，数据部分是当前进程的 PID
    auto wait_resource_id = ResourceId(ResourceType::kChildExit, current->pid);

    Block(wait_resource_id);

    klog::Debug("Wait: pid={} blocked on resource={}, data={}", current->pid,
                wait_resource_id.GetTypeName(),
                static_cast<uint64_t>(wait_resource_id.GetData()));
    // 被唤醒后重新检查
  }
}

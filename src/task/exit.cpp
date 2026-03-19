/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>

#include "kernel_log.hpp"
#include "resource_id.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

auto TaskManager::Exit(int exit_code) -> void {
  auto& cpu_sched = GetCurrentCpuSched();
  auto* current = GetCurrentTask();
  assert(current != nullptr && "Exit: No current task to exit");
  assert(current->GetStatus() == TaskStatus::kRunning &&
         "Exit: current task status must be kRunning");

  {
    LockGuard<SpinLock> lock_guard(cpu_sched.lock);

    current->aux->exit_code = exit_code;
    bool is_group_leader = current->IsThreadGroupLeader();

    if (is_group_leader && current->GetThreadGroupSize() > 1) {
      klog::Warn(
          "Exit: Thread group leader (pid={}, tgid={}) exiting, but group "
          "still has {} threads",
          current->pid, current->aux->tgid, current->GetThreadGroupSize());
    }

    current->LeaveThreadGroup();

    if (current->aux->parent_pid != 0) {
      current->fsm.Receive(MsgExit{exit_code, true});

      auto wait_resource_id =
          ResourceId(ResourceType::kChildExit, current->aux->parent_pid);
      Wakeup(cpu_sched, wait_resource_id);

      klog::Debug("Exit: pid={} waking up parent={} on resource={}",
                  current->pid, current->aux->parent_pid,
                  wait_resource_id.GetTypeName());
    } else {
      current->fsm.Receive(MsgExit{exit_code, false});
    }

    if (is_group_leader) {
      ReparentChildren(current);
    }

    ReapTask(current);
  }

  Schedule();

  klog::Err("Exit: Task {} should not return from Schedule()", current->pid);

  // UNREACHABLE: 任务退出后 Schedule() 不应返回
  __builtin_unreachable();
}

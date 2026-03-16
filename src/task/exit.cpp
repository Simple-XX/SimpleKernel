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

    // 设置退出码
    current->aux->exit_code = exit_code;

    // 检查是否是线程组的主线程
    bool is_group_leader = current->IsThreadGroupLeader();

    // 如果是线程组的主线程，需要检查是否还有其他线程
    if (is_group_leader && current->GetThreadGroupSize() > 1) {
      klog::Warn(
          "Exit: Thread group leader (pid={}, tgid={}) exiting, but group "
          "still has {} threads",
          current->pid, current->aux->tgid, current->GetThreadGroupSize());
    }

    // 从线程组中移除
    current->LeaveThreadGroup();

    // 将子进程过继给 init 进程 (仅当是进程时)
    if (is_group_leader) {
      ReparentChildren(current);
    }

    if (current->aux->parent_pid != 0) {
      // 有父进程，进入僵尸状态等待回收
      // Transition: kRunning -> kZombie
      current->fsm.Receive(MsgExit{exit_code, true});

      // 唤醒等待此进程退出的父进程
      // 父进程会阻塞在 ChildExit 类型的资源上，数据是父进程自己的 PID
      auto wait_resource_id =
          ResourceId(ResourceType::kChildExit, current->aux->parent_pid);
      Wakeup(wait_resource_id);

      /// @todo 通知父进程 (发送 SIGCHLD)

      klog::Debug("Exit: pid={} waking up parent={} on resource={}",
                  current->pid, current->aux->parent_pid,
                  wait_resource_id.GetTypeName());
    } else {
      // 没有父进程，直接退出并释放资源
      // Transition: kRunning -> kExited
      current->fsm.Receive(MsgExit{exit_code, false});
      // No parent to call wait(), reap immediately to free TCB + stack
      ReapTask(current);
    }
  }

  Schedule();

  klog::Err("Exit: Task {} should not return from Schedule()", current->pid);

  // UNREACHABLE: 任务退出后 Schedule() 不应返回
  __builtin_unreachable();
}

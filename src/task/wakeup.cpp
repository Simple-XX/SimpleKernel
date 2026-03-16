/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>

#include "kernel_log.hpp"
#include "resource_id.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

auto TaskManager::Wakeup(ResourceId resource_id) -> void {
  auto& cpu_sched = GetCurrentCpuSched();

  LockGuard<SpinLock> lock_guard(cpu_sched.lock);

  // 查找等待该资源的任务列表
  auto it = cpu_sched.blocked_tasks.find(resource_id);

  if (it == cpu_sched.blocked_tasks.end()) {
    // 没有任务等待该资源
    klog::Debug("Wakeup: No tasks waiting on resource={}, data={:#x}",
                resource_id.GetTypeName(),
                static_cast<uint64_t>(resource_id.GetData()));
    return;
  }

  // 唤醒所有等待该资源的任务
  auto& waiting_tasks = it->second;
  size_t wakeup_count = 0;

  while (!waiting_tasks.empty()) {
    auto* task = waiting_tasks.front();
    waiting_tasks.pop_front();

    assert(task->GetStatus() == TaskStatus::kBlocked &&
           "Wakeup: task status must be kBlocked");
    assert(task->aux->blocked_on == resource_id &&
           "Wakeup: task blocked_on must match resource_id");

    // 将任务标记为就绪
    task->fsm.Receive(MsgWakeup{});
    task->aux->blocked_on = ResourceId{};

    // 将任务重新加入对应调度器的就绪队列
    auto* scheduler =
        cpu_sched.schedulers[static_cast<uint8_t>(task->policy)].get();
    assert(scheduler != nullptr && "Wakeup: scheduler must not be null");
    scheduler->Enqueue(task);
    wakeup_count++;
  }

  // 移除空的资源队列
  cpu_sched.blocked_tasks.erase(resource_id);

  klog::Debug("Wakeup: Woke up {} tasks from resource={}, data={:#x}",
              wakeup_count, resource_id.GetTypeName(),
              static_cast<uint64_t>(resource_id.GetData()));
}

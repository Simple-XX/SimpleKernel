/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>

#include "kernel_log.hpp"
#include "resource_id.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

auto TaskManager::Wakeup(CpuSchedData& cpu_sched, ResourceId resource_id)
    -> void {
  auto it = cpu_sched.blocked_tasks.find(resource_id);

  if (it == cpu_sched.blocked_tasks.end()) {
    klog::Debug("Wakeup: No tasks waiting on resource={}, data={:#x}",
                resource_id.GetTypeName(),
                static_cast<uint64_t>(resource_id.GetData()));
    return;
  }

  auto& waiting_tasks = it->second;
  size_t wakeup_count = 0;

  while (!waiting_tasks.empty()) {
    auto* task = waiting_tasks.front();
    waiting_tasks.pop_front();

    assert(task->GetStatus() == TaskStatus::kBlocked &&
           "Wakeup: task status must be kBlocked");
    assert(task->aux->blocked_on == resource_id &&
           "Wakeup: task blocked_on must match resource_id");

    task->fsm.Receive(MsgWakeup{});
    task->aux->blocked_on = ResourceId{};

    auto* scheduler =
        cpu_sched.schedulers[static_cast<uint8_t>(task->policy)].get();
    assert(scheduler != nullptr && "Wakeup: scheduler must not be null");
    scheduler->Enqueue(task);
    wakeup_count++;
  }

  cpu_sched.blocked_tasks.erase(resource_id);

  klog::Debug("Wakeup: Woke up {} tasks from resource={}, data={:#x}",
              wakeup_count, resource_id.GetTypeName(),
              static_cast<uint64_t>(resource_id.GetData()));
}

auto TaskManager::Wakeup(ResourceId resource_id) -> void {
  auto& cpu_sched = GetCurrentCpuSched();
  LockGuard<SpinLock> lock_guard(cpu_sched.lock);
  Wakeup(cpu_sched, resource_id);
}

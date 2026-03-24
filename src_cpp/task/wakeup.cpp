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
  for (size_t i = 0; i < SIMPLEKERNEL_MAX_CORE_COUNT; ++i) {
    auto& cpu_sched = cpu_schedulers_[i];
    LockGuard<SpinLock> lock_guard(cpu_sched.lock);
    Wakeup(cpu_sched, resource_id);
  }
}

auto TaskManager::WakeupOne(ResourceId resource_id) -> void {
  for (size_t i = 0; i < SIMPLEKERNEL_MAX_CORE_COUNT; ++i) {
    auto& cpu_sched = cpu_schedulers_[i];
    LockGuard<SpinLock> lock_guard(cpu_sched.lock);

    auto it = cpu_sched.blocked_tasks.find(resource_id);
    if (it == cpu_sched.blocked_tasks.end()) {
      continue;
    }

    auto& waiting_tasks = it->second;
    if (waiting_tasks.empty()) {
      continue;
    }

    auto* task = waiting_tasks.front();
    waiting_tasks.pop_front();

    assert(task->GetStatus() == TaskStatus::kBlocked &&
           "WakeupOne: task status must be kBlocked");

    task->fsm.Receive(MsgWakeup{});
    task->aux->blocked_on = ResourceId{};

    auto* scheduler =
        cpu_sched.schedulers[static_cast<uint8_t>(task->policy)].get();
    assert(scheduler != nullptr && "WakeupOne: scheduler must not be null");
    scheduler->Enqueue(task);

    if (waiting_tasks.empty()) {
      cpu_sched.blocked_tasks.erase(resource_id);
    }

    klog::Debug("WakeupOne: Woke task pid={} from resource={}, data={:#x}",
                task->pid, resource_id.GetTypeName(),
                static_cast<uint64_t>(resource_id.GetData()));
    return;  // Only wake one
  }

  klog::Debug("WakeupOne: No tasks waiting on resource={}, data={:#x}",
              resource_id.GetTypeName(),
              static_cast<uint64_t>(resource_id.GetData()));
}

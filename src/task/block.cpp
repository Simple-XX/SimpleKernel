/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cassert>

#include "kernel_log.hpp"
#include "resource_id.hpp"
#include "task_manager.hpp"
#include "task_messages.hpp"

auto TaskManager::Block(CpuSchedData& cpu_sched, ResourceId resource_id)
    -> void {
  auto* current = GetCurrentTask();
  assert(current != nullptr && "Block: No current task to block");
  assert(current->GetStatus() == TaskStatus::kRunning &&
         "Block: current task status must be kRunning");

  auto& list = cpu_sched.blocked_tasks[resource_id];
  if (list.full()) {
    klog::Err(
        "Block: blocked_tasks list full for resource, cannot block task {}",
        current->pid);
    return;
  }

  current->fsm.Receive(MsgBlock{resource_id});
  current->aux->blocked_on = resource_id;
  list.push_back(current);

  klog::Debug("Block: pid={} blocked on resource={}, data={:#x}", current->pid,
              resource_id.GetTypeName(),
              static_cast<uint64_t>(resource_id.GetData()));
}

auto TaskManager::Block(ResourceId resource_id) -> void {
  auto& cpu_sched = GetCurrentCpuSched();
  {
    LockGuard<SpinLock> lock_guard(cpu_sched.lock);
    Block(cpu_sched, resource_id);
  }
  Schedule();
}

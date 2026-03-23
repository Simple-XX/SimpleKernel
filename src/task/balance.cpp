/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include "kernel_log.hpp"
#include "task_manager.hpp"

auto TaskManager::Balance() -> void {
  auto current_core = cpu_io::GetCurrentCoreId();
  auto& current_sched = cpu_schedulers_[current_core];

  // 获取当前核心 kNormal 队列长度（无锁快速检查）
  size_t current_load = 0;
  if (current_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]) {
    current_load =
        current_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]
            ->GetQueueSize();
  }

  // 寻找负载最高的核心
  size_t max_load = 0;
  size_t max_core = current_core;

  for (size_t core_id = 0; core_id < SIMPLEKERNEL_MAX_CORE_COUNT; ++core_id) {
    if (core_id == current_core) {
      continue;
    }
    auto& other_sched = cpu_schedulers_[core_id];
    if (other_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]) {
      size_t load =
          other_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]
              ->GetQueueSize();
      if (load > max_load) {
        max_load = load;
        max_core = core_id;
      }
    }
  }

  // 仅当差值 > 1 时才窃取（避免 ping-pong）
  if (max_core == current_core || max_load <= current_load + 1) {
    return;
  }

  // 按核心 ID 顺序获取锁，防止死锁
  auto& source_sched = cpu_schedulers_[max_core];
  size_t first_core = (current_core < max_core) ? current_core : max_core;
  size_t second_core = (current_core < max_core) ? max_core : current_core;

  LockGuard<SpinLock> lock_first(cpu_schedulers_[first_core].lock);
  LockGuard<SpinLock> lock_second(cpu_schedulers_[second_core].lock);

  // 重新检查（持锁后条件可能已变化）
  auto* source_scheduler =
      source_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)].get();
  auto* dest_scheduler =
      current_sched.schedulers[static_cast<uint8_t>(SchedPolicy::kNormal)]
          .get();

  if (!source_scheduler || !dest_scheduler) {
    return;
  }

  size_t source_load = source_scheduler->GetQueueSize();
  size_t dest_load = dest_scheduler->GetQueueSize();

  if (source_load <= dest_load + 1) {
    return;
  }

  auto* stolen = source_scheduler->PickNext();
  if (!stolen) {
    return;
  }

  if (stolen->aux && stolen->aux->cpu_affinity.value() != UINT64_MAX &&
      !(stolen->aux->cpu_affinity.value() & (1UL << current_core))) {
    source_scheduler->Enqueue(stolen);
    return;
  }

  dest_scheduler->Enqueue(stolen);
  klog::Debug("Balance: Stole task '{}' (pid={}) from core {} to core {}",
              stolen->name, stolen->pid, max_core, current_core);
}

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>

#include "basic_info.hpp"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "per_cpu.hpp"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

// ===========================================================================
// test_balance_imbalanced_load
//
// AddTask() places unpinned tasks on the caller's current core, so spawning
// many tasks from one core creates a natural imbalance. Balance() (called
// every 64 ticks) should steal tasks to idle cores.
//
// Strategy:
//   1. Spawn N long-lived unpinned workers (they sleep in a loop so Balance()
//      has time to migrate them).
//   2. Each worker periodically records which core it runs on via atomic OR.
//   3. After all workers complete, verify the combined core mask has >= 2 bits
//      set — meaning at least one task was migrated by Balance().
//
// Workers must live long enough for multiple Balance() intervals (64 ticks
// each). Using sys_sleep(50) in a loop ensures they remain in the scheduler
// queues across multiple ticks.
// ===========================================================================

constexpr int kImbalanceWorkerCount = 8;
std::atomic<int> g_imbalance_done{0};
std::atomic<uint64_t> g_cores_used_mask{0};

void imbalance_worker(void* /*arg*/) {
  // Stay alive across multiple Balance() intervals.
  // Record core ID on each wakeup — after migration we'll see a different
  // core.
  for (int i = 0; i < 20; ++i) {
    auto core_id = cpu_io::GetCurrentCoreId();
    g_cores_used_mask.fetch_or(1UL << core_id, std::memory_order_relaxed);
    (void)sys_sleep(10);
  }

  g_imbalance_done.fetch_add(1, std::memory_order_release);
  sys_exit(0);
}

void test_balance_imbalanced_load(void* /*arg*/) {
  klog::Info("=== Balance: Imbalanced Load Test ===");

  g_imbalance_done = 0;
  g_cores_used_mask = 0;
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  // All workers are unpinned (default affinity = UINT64_MAX).
  // AddTask() places them on the caller's current core, creating a heavy
  // imbalance that Balance() should correct.
  for (int i = 0; i < kImbalanceWorkerCount; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>("BalWorker", 10,
                                                    imbalance_worker, nullptr);
    task->aux->parent_pid = self->pid;
    task->aux->pgid = self->aux->pgid;
    tm.AddTask(std::move(task));
  }

  // Wait for all workers to finish
  int timeout = 600;
  while (timeout > 0 && g_imbalance_done.load(std::memory_order_acquire) <
                            kImbalanceWorkerCount) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (g_imbalance_done.load() != kImbalanceWorkerCount) {
    klog::Err(
        "test_balance_imbalanced_load: FAIL — only {}/{} workers finished",
        g_imbalance_done.load(), kImbalanceWorkerCount);
    passed = false;
  }

  // With 8 workers all initially on one core and Balance() active,
  // tasks should have been migrated to run on more than one core.
  uint64_t mask = g_cores_used_mask.load(std::memory_order_acquire);
  int cores_used = __builtin_popcountll(mask);
  if (cores_used < 2) {
    klog::Err(
        "test_balance_imbalanced_load: FAIL — tasks only used {} core(s), "
        "expected >= 2 (mask={:#x})",
        cores_used, mask);
    passed = false;
  } else {
    klog::Info(
        "test_balance_imbalanced_load: tasks used {} core(s) (mask={:#x})",
        cores_used, mask);
  }

  if (passed) {
    klog::Info("Balance: Imbalanced Load Test: PASSED");
  } else {
    klog::Err("Balance: Imbalanced Load Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

// ===========================================================================
// test_balance_respects_affinity
//
// Pin all tasks to core 0. Even though core 1 is idle, Balance() must NOT
// migrate pinned tasks. Each worker records its core on every wakeup;
// the combined mask must only contain core 0.
// ===========================================================================

constexpr int kAffinityWorkerCount = 4;
std::atomic<int> g_affinity_done{0};
std::atomic<uint64_t> g_affinity_cores_mask{0};

void affinity_pinned_worker(void* /*arg*/) {
  for (int i = 0; i < 10; ++i) {
    auto core_id = cpu_io::GetCurrentCoreId();
    g_affinity_cores_mask.fetch_or(1UL << core_id, std::memory_order_relaxed);
    (void)sys_sleep(10);
  }

  g_affinity_done.fetch_add(1, std::memory_order_release);
  sys_exit(0);
}

void test_balance_respects_affinity(void* /*arg*/) {
  klog::Info("=== Balance: Respects Affinity Test ===");

  g_affinity_done = 0;
  g_affinity_cores_mask = 0;
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  // Pin all workers to core 0
  for (int i = 0; i < kAffinityWorkerCount; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>(
        "BalPinned", 10, affinity_pinned_worker, nullptr);
    task->aux->parent_pid = self->pid;
    task->aux->pgid = self->aux->pgid;
    task->aux->cpu_affinity = (1UL << 0);
    tm.AddTask(std::move(task));
  }

  // Wait for all workers
  int timeout = 400;
  while (timeout > 0 && g_affinity_done.load(std::memory_order_acquire) <
                            kAffinityWorkerCount) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (g_affinity_done.load() != kAffinityWorkerCount) {
    klog::Err(
        "test_balance_respects_affinity: FAIL — only {}/{} workers finished",
        g_affinity_done.load(), kAffinityWorkerCount);
    passed = false;
  }

  // All tasks pinned to core 0 should only have run on core 0
  uint64_t mask = g_affinity_cores_mask.load(std::memory_order_acquire);
  if (mask != (1UL << 0)) {
    klog::Err(
        "test_balance_respects_affinity: FAIL — pinned tasks ran on "
        "cores {:#x}, expected only core 0 (0x1)",
        mask);
    passed = false;
  }

  if (passed) {
    klog::Info("Balance: Respects Affinity Test: PASSED");
  } else {
    klog::Err("Balance: Respects Affinity Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

}  // namespace

auto balance_test() -> bool {
  klog::Info("===== Balance System Test Start =====");

  auto core_count = BasicInfoSingleton::instance().core_count;
  if (core_count < 2) {
    klog::Info("Skipping balance tests: need >= 2 cores, have {}", core_count);
    return true;
  }

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();

  auto t1 = kstd::make_unique<TaskControlBlock>(
      "TestBalImbalance", 10, test_balance_imbalanced_load, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>(
      "TestBalAffinity", 10, test_balance_respects_affinity, nullptr);
  tm.AddTask(std::move(t2));

  constexpr int kExpectedTests = 2;
  int timeout = 600;
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed.load() >= kExpectedTests) {
      break;
    }
    timeout--;
  }

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All balance sub-tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No balance sub-tests failed");

  klog::Info("===== Balance System Test End =====");
  return true;
}

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>

#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

// ===========================================================================
// test_affinity_get_self
//
// Verify that the default affinity for the current task is UINT64_MAX.
// ===========================================================================

void test_affinity_get_self(void* /*arg*/) {
  klog::Info("=== Affinity Get Self Test ===");

  bool passed = true;
  uint64_t mask = 0;

  int ret = sys_sched_getaffinity(0, sizeof(uint64_t), &mask);
  if (ret != 0) {
    klog::Err("test_affinity_get_self: getaffinity returned {}, expected 0",
              ret);
    passed = false;
  }

  if (passed && mask != UINT64_MAX) {
    klog::Err("test_affinity_get_self: mask={:#x}, expected {:#x}", mask,
              static_cast<uint64_t>(UINT64_MAX));
    passed = false;
  }

  if (passed) {
    klog::Info("Affinity Get Self Test: PASSED");
  } else {
    klog::Err("Affinity Get Self Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

// ===========================================================================
// test_affinity_set_get
//
// Set affinity to 1UL, read back and verify, then restore to UINT64_MAX.
// ===========================================================================

void test_affinity_set_get(void* /*arg*/) {
  klog::Info("=== Affinity Set/Get Test ===");

  bool passed = true;

  // Set affinity to CPU 0 only
  uint64_t new_mask = 1UL;
  int ret = sys_sched_setaffinity(0, sizeof(uint64_t), &new_mask);
  if (ret != 0) {
    klog::Err("test_affinity_set_get: setaffinity returned {}, expected 0",
              ret);
    passed = false;
  }

  // Read back and verify
  uint64_t read_mask = 0;
  if (passed) {
    ret = sys_sched_getaffinity(0, sizeof(uint64_t), &read_mask);
    if (ret != 0) {
      klog::Err("test_affinity_set_get: getaffinity returned {}, expected 0",
                ret);
      passed = false;
    }
  }

  if (passed && read_mask != 1UL) {
    klog::Err("test_affinity_set_get: read_mask={:#x}, expected 0x1",
              read_mask);
    passed = false;
  }

  // Restore default affinity
  uint64_t all_mask = UINT64_MAX;
  (void)sys_sched_setaffinity(0, sizeof(uint64_t), &all_mask);

  if (passed) {
    klog::Info("Affinity Set/Get Test: PASSED");
  } else {
    klog::Err("Affinity Set/Get Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

// ===========================================================================
// test_affinity_other_task
//
// Spawn a sleeping task and read its affinity by PID — expect UINT64_MAX.
// ===========================================================================

std::atomic<Pid> g_sleeper_pid{0};

void sleeper_work(void* /*arg*/) {
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  g_sleeper_pid.store(current->pid);
  (void)sys_sleep(5000);
  sys_exit(0);
}

void test_affinity_other_task(void* /*arg*/) {
  klog::Info("=== Affinity Other Task Test ===");

  bool passed = true;
  g_sleeper_pid = 0;

  // Spawn a sleeping task
  auto sleeper = kstd::make_unique<TaskControlBlock>("AffinitySleeper", 10,
                                                     sleeper_work, nullptr);
  TaskManagerSingleton::instance().AddTask(std::move(sleeper));

  // Wait for the sleeper to start and publish its PID
  int timeout = 100;
  while (timeout > 0 && g_sleeper_pid.load() == 0) {
    (void)sys_sleep(10);
    timeout--;
  }

  Pid other_pid = g_sleeper_pid.load();
  if (other_pid == 0) {
    klog::Err("test_affinity_other_task: sleeper did not start");
    passed = false;
  }

  // Read the sleeper's affinity by PID
  uint64_t mask = 0;
  if (passed) {
    int ret = sys_sched_getaffinity(static_cast<int>(other_pid),
                                    sizeof(uint64_t), &mask);
    if (ret != 0) {
      klog::Err(
          "test_affinity_other_task: getaffinity(pid={}) returned {}, "
          "expected 0",
          other_pid, ret);
      passed = false;
    }
  }

  if (passed && mask != UINT64_MAX) {
    klog::Err("test_affinity_other_task: mask={:#x}, expected {:#x}", mask,
              static_cast<uint64_t>(UINT64_MAX));
    passed = false;
  }

  if (passed) {
    klog::Info("Affinity Other Task Test: PASSED");
  } else {
    klog::Err("Affinity Other Task Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

// ===========================================================================
// test_affinity_errors
//
// Verify error returns for invalid PID and too-small cpusetsize.
// ===========================================================================

void test_affinity_errors(void* /*arg*/) {
  klog::Info("=== Affinity Errors Test ===");

  bool passed = true;
  uint64_t mask = 0;

  // Invalid PID should return -1
  int ret = sys_sched_getaffinity(99999, sizeof(uint64_t), &mask);
  if (ret != -1) {
    klog::Err(
        "test_affinity_errors: getaffinity(pid=99999) returned {}, "
        "expected -1",
        ret);
    passed = false;
  }

  // Too-small cpusetsize should return -1
  ret = sys_sched_getaffinity(0, 1, &mask);
  if (ret != -1) {
    klog::Err(
        "test_affinity_errors: getaffinity(cpusetsize=1) returned {}, "
        "expected -1",
        ret);
    passed = false;
  }

  if (passed) {
    klog::Info("Affinity Errors Test: PASSED");
  } else {
    klog::Err("Affinity Errors Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto affinity_test() -> bool {
  klog::Info("===== Affinity System Test Start =====");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& task_mgr = TaskManagerSingleton::instance();

  auto test1 = kstd::make_unique<TaskControlBlock>(
      "TestAffinityGetSelf", 10, test_affinity_get_self, nullptr);
  task_mgr.AddTask(std::move(test1));

  auto test2 = kstd::make_unique<TaskControlBlock>(
      "TestAffinitySetGet", 10, test_affinity_set_get, nullptr);
  task_mgr.AddTask(std::move(test2));

  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestAffinityOther", 10, test_affinity_other_task, nullptr);
  task_mgr.AddTask(std::move(test3));

  auto test4 = kstd::make_unique<TaskControlBlock>(
      "TestAffinityErrors", 10, test_affinity_errors, nullptr);
  task_mgr.AddTask(std::move(test4));

  int timeout = 200;
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed >= 4) {
      break;
    }
    timeout--;
  }

  EXPECT_EQ(g_tests_completed, 4, "tests completed");
  EXPECT_EQ(g_tests_failed, 0, "tests failed");

  klog::Info("Affinity System Test Suite: COMPLETED");
  return true;
}

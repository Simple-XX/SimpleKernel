/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>

#include "kernel.h"
#include "kernel_config.hpp"
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

// ---------------------------------------------------------------------------
// test_many_tasks
// Spawn 20 tasks simultaneously, each increments an atomic counter and exits.
// Parent waits for all by PID, then verifies counter == 20.
// ---------------------------------------------------------------------------

std::atomic<int> g_many_tasks_counter{0};

void many_tasks_work(void* /*arg*/) {
  g_many_tasks_counter++;
  sys_exit(0);
}

void test_many_tasks(void* /*arg*/) {
  klog::Info("=== Stress: Many Tasks Test ===");

  g_many_tasks_counter = 0;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  if (!self) {
    klog::Err("test_many_tasks: Cannot get current task");
    g_tests_failed++;
    g_tests_completed++;
    sys_exit(1);
  }

  constexpr int kTaskCount = 20;
  Pid pids[kTaskCount];

  // Spawn all 20 tasks
  for (int i = 0; i < kTaskCount; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>("StressWorker", 10,
                                                    many_tasks_work, nullptr);
    task->aux->parent_pid = self->pid;
    task->aux->pgid = self->aux->pgid;
    auto* raw = task.get();
    tm.AddTask(std::move(task));
    pids[i] = raw->pid;
  }

  // Wait for each task by PID
  bool passed = true;
  for (int i = 0; i < kTaskCount; ++i) {
    int status = 0;
    int timeout = 100;
    while (timeout-- > 0) {
      auto result = tm.Wait(pids[i], &status, false, false);
      if (result.has_value() && result.value() == pids[i]) {
        break;
      }
      (void)sys_sleep(10);
    }
    if (timeout <= 0) {
      klog::Err("test_many_tasks: timed out waiting for task {}", pids[i]);
      passed = false;
    }
  }

  if (g_many_tasks_counter.load() != kTaskCount) {
    klog::Err("test_many_tasks: FAIL - counter={}, expected {}",
              g_many_tasks_counter.load(), kTaskCount);
    passed = false;
  }

  if (passed) {
    klog::Info("Stress: Many Tasks Test: PASSED");
  } else {
    klog::Err("Stress: Many Tasks Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_wait_non_child
// Call Wait with a bogus PID that is not a child of the current task.
// Expect the call to fail (no value, or value <= 0).
// ---------------------------------------------------------------------------

void test_wait_non_child(void* /*arg*/) {
  klog::Info("=== Stress: Wait Non-Child Test ===");

  auto& tm = TaskManagerSingleton::instance();

  int status = 0;
  auto result = tm.Wait(99999, &status, false, false);

  bool passed = true;
  if (result.has_value() && result.value() > 0) {
    klog::Err(
        "test_wait_non_child: FAIL - Wait(99999) should fail but got pid={}",
        result.value());
    passed = false;
  } else {
    klog::Info("test_wait_non_child: Wait(99999) correctly failed");
  }

  if (passed) {
    klog::Info("Stress: Wait Non-Child Test: PASSED");
  } else {
    klog::Err("Stress: Wait Non-Child Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_rapid_create_exit
// Loop 10 times: create a task that exits with code i, Wait for it,
// verify exit code == i. Tests rapid lifecycle churn.
// ---------------------------------------------------------------------------

void rapid_exit_work(void* arg) {
  int code = static_cast<int>(reinterpret_cast<uint64_t>(arg));
  sys_exit(code);
}

void test_rapid_create_exit(void* /*arg*/) {
  klog::Info("=== Stress: Rapid Create/Exit Test ===");

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  if (!self) {
    klog::Err("test_rapid_create_exit: Cannot get current task");
    g_tests_failed++;
    g_tests_completed++;
    sys_exit(1);
  }

  bool passed = true;
  constexpr int kIterations = 10;

  for (int i = 0; i < kIterations; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>(
        "RapidExit", 10, rapid_exit_work, reinterpret_cast<void*>(i));
    task->aux->parent_pid = self->pid;
    task->aux->pgid = self->aux->pgid;
    auto* raw = task.get();
    tm.AddTask(std::move(task));
    Pid pid = raw->pid;

    int status = 0;
    int timeout = 100;
    while (timeout-- > 0) {
      auto result = tm.Wait(pid, &status, false, false);
      if (result.has_value() && result.value() == pid) {
        break;
      }
      (void)sys_sleep(10);
    }

    if (timeout <= 0) {
      klog::Err("test_rapid_create_exit: timed out waiting for task {}", pid);
      passed = false;
      break;
    }

    if (status != i) {
      klog::Err(
          "test_rapid_create_exit: FAIL - iteration {}: exit code={}, "
          "expected {}",
          i, status, i);
      passed = false;
    }
  }

  if (passed) {
    klog::Info("Stress: Rapid Create/Exit Test: PASSED");
  } else {
    klog::Err("Stress: Rapid Create/Exit Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

}  // namespace

/**
 * @brief Stress system test entry point
 */
auto stress_test() -> bool {
  klog::Info("===== Stress System Test Start =====");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();

  // Sub-test 1: Many tasks
  auto t1 = kstd::make_unique<TaskControlBlock>("TestManyTasks", 10,
                                                test_many_tasks, nullptr);
  tm.AddTask(std::move(t1));

  // Sub-test 2: Wait non-child
  auto t2 = kstd::make_unique<TaskControlBlock>("TestWaitNonChild", 10,
                                                test_wait_non_child, nullptr);
  tm.AddTask(std::move(t2));

  // Sub-test 3: Rapid create/exit
  auto t3 = kstd::make_unique<TaskControlBlock>(
      "TestRapidCreateExit", 10, test_rapid_create_exit, nullptr);
  tm.AddTask(std::move(t3));

  // Wait for all 3 sub-tests to complete (timeout: 400 * 50ms = 20s)
  constexpr int kExpectedTests = 3;
  int timeout = 400;
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed.load() >= kExpectedTests) {
      break;
    }
    timeout--;
  }

  klog::Info("Stress System Test: completed={}, failed={}",
             g_tests_completed.load(), g_tests_failed.load());

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All 3 stress sub-tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No stress sub-tests failed");

  klog::Info("===== Stress System Test End =====");
  return true;
}

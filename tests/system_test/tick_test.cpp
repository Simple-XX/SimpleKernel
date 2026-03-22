/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>

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

void test_tick_increments(void*) {
  klog::Info("=== Tick Increments Test ===");

  auto* sched_data = per_cpu::GetCurrentCore().sched_data;
  uint64_t tick_before = sched_data->local_tick;

  (void)sys_sleep(200);

  uint64_t tick_after = sched_data->local_tick;

  if (tick_after <= tick_before) {
    klog::Err(
        "test_tick_increments: FAIL -- local_tick did not increase "
        "(before={}, after={})",
        tick_before, tick_after);
    g_tests_failed++;
  } else {
    klog::Info("Tick Increments Test: PASSED (before={}, after={}, delta={})",
               tick_before, tick_after, tick_after - tick_before);
  }

  g_tests_completed++;
  sys_exit(0);
}

void test_sleep_timing(void*) {
  klog::Info("=== Sleep Timing Test ===");

  auto* sched_data = per_cpu::GetCurrentCore().sched_data;
  uint64_t tick_before = sched_data->local_tick;

  (void)sys_sleep(500);

  uint64_t tick_after = sched_data->local_tick;
  uint64_t delta = tick_after - tick_before;

  if (delta == 0) {
    klog::Err(
        "test_sleep_timing: FAIL -- tick delta is 0 after 500ms sleep "
        "(before={}, after={})",
        tick_before, tick_after);
    g_tests_failed++;
  } else {
    klog::Info("Sleep Timing Test: PASSED (before={}, after={}, delta={})",
               tick_before, tick_after, delta);
  }

  g_tests_completed++;
  sys_exit(0);
}

void test_runtime_tracking(void*) {
  klog::Info("=== Runtime Tracking Test ===");

  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  uint64_t runtime_before = self->sched_info.total_runtime;

  // Busy-wait for at least 5 tick intervals to ensure total_runtime
  // is incremented (1ms per tick at SIMPLEKERNEL_TICK=1000)
  auto* sched_data = per_cpu::GetCurrentCore().sched_data;
  uint64_t start_tick = sched_data->local_tick;
  volatile uint64_t sink = 0;
  while (sched_data->local_tick - start_tick < 5) {
    sink = sink + 1;
  }
  (void)sink;

  uint64_t runtime_after = self->sched_info.total_runtime;

  if (runtime_after <= runtime_before) {
    klog::Err(
        "test_runtime_tracking: FAIL -- total_runtime did not increase "
        "(before={}, after={})",
        runtime_before, runtime_after);
    g_tests_failed++;
  } else {
    klog::Info("Runtime Tracking Test: PASSED (before={}, after={}, delta={})",
               runtime_before, runtime_after, runtime_after - runtime_before);
  }

  g_tests_completed++;
  sys_exit(0);
}

}  // namespace

auto tick_test() -> bool {
  klog::Info("=== Tick System Test Suite ===");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& task_mgr = TaskManagerSingleton::instance();

  auto test1 = kstd::make_unique<TaskControlBlock>(
      "TestTickIncrements", 10, test_tick_increments, nullptr);
  task_mgr.AddTask(std::move(test1));

  auto test2 = kstd::make_unique<TaskControlBlock>("TestSleepTiming", 10,
                                                   test_sleep_timing, nullptr);
  task_mgr.AddTask(std::move(test2));

  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestRuntimeTracking", 10, test_runtime_tracking, nullptr);
  task_mgr.AddTask(std::move(test3));

  constexpr int kExpectedTests = 3;
  int timeout = 400;
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed >= kExpectedTests) {
      break;
    }
    timeout--;
  }

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All tick tests should complete");
  EXPECT_EQ(g_tests_failed.load(), 0, "No tick tests should fail");

  klog::Info("Tick System Test Suite: COMPLETED");
  return true;
}

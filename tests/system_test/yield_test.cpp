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

/// Test 1: sys_yield returns 0
std::atomic<int> g_yield_ret{-1};

void yield_basic_work(void* /*arg*/) {
  int ret = sys_yield();
  g_yield_ret.store(ret);
  sys_exit(0);
}

/// Test 2: yield preserves execution order fairness
std::atomic<int> g_a_count{0};
std::atomic<int> g_b_count{0};
std::atomic<bool> g_fairness_done{false};

void yield_task_a(void* /*arg*/) {
  for (int i = 0; i < 10; ++i) {
    g_a_count++;
    (void)sys_yield();
  }
  sys_exit(0);
}

void yield_task_b(void* /*arg*/) {
  for (int i = 0; i < 10; ++i) {
    g_b_count++;
    (void)sys_yield();
  }
  g_fairness_done.store(true);
  sys_exit(0);
}

}  // namespace

auto yield_test() -> bool {
  klog::Info("yield_test: start");
  auto& tm = TaskManagerSingleton::instance();

  // Test 1: basic yield returns 0
  g_yield_ret = -1;
  auto t1 = kstd::make_unique<TaskControlBlock>("YieldBasic", 10,
                                                yield_basic_work, nullptr);
  tm.AddTask(std::move(t1));

  int timeout = 100;
  while (timeout-- > 0 && g_yield_ret.load() == -1) {
    (void)sys_sleep(50);
  }
  EXPECT_EQ(g_yield_ret.load(), 0, "sys_yield should return 0");

  // Test 2: yield fairness — both tasks make progress
  g_a_count = 0;
  g_b_count = 0;
  g_fairness_done = false;

  auto ta =
      kstd::make_unique<TaskControlBlock>("YieldA", 10, yield_task_a, nullptr);
  auto tb =
      kstd::make_unique<TaskControlBlock>("YieldB", 10, yield_task_b, nullptr);
  tm.AddTask(std::move(ta));
  tm.AddTask(std::move(tb));

  timeout = 100;
  while (timeout-- > 0 && !g_fairness_done.load()) {
    (void)sys_sleep(50);
  }
  EXPECT_EQ(g_a_count.load(), 10, "Task A should complete 10 iterations");
  EXPECT_EQ(g_b_count.load(), 10, "Task B should complete 10 iterations");

  klog::Info("yield_test: PASS");
  return true;
}

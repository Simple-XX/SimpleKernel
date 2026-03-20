/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "basic_info.hpp"
#include "kernel.h"
#include "kstd_memory"
#include "mutex.hpp"
#include "per_cpu.hpp"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

// ===========================================================================
// test_cross_core_wakeup
//
// Two tasks pinned to different cores share a Mutex.
// Task A (core 0) acquires it, sleeps 200ms, releases.
// Task B (core 1) blocks on Lock(), wakes up when A releases.
// Verify B acquired the mutex after A released it.
// ===========================================================================

struct WakeupArgs {
  Mutex* mtx;
  std::atomic<int>* phase;  // 0:init, 1:A locked, 2:A unlocked, 3:B locked
};

void wakeup_task_a(void* arg) {
  auto* ctx = reinterpret_cast<WakeupArgs*>(arg);

  (void)ctx->mtx->Lock();
  ctx->phase->store(1, std::memory_order_release);
  klog::Info("[CrossCore Wakeup] A: locked mutex, sleeping 200ms");

  (void)sys_sleep(200);

  (void)ctx->mtx->UnLock();
  ctx->phase->store(2, std::memory_order_release);
  klog::Info("[CrossCore Wakeup] A: unlocked mutex");

  sys_exit(0);
}

void wakeup_task_b(void* arg) {
  auto* ctx = reinterpret_cast<WakeupArgs*>(arg);

  // Wait until A has locked the mutex before trying
  int timeout = 100;
  while (timeout > 0 && ctx->phase->load(std::memory_order_acquire) < 1) {
    (void)sys_sleep(10);
    timeout--;
  }

  klog::Info("[CrossCore Wakeup] B: attempting Lock() (should block)");
  (void)ctx->mtx->Lock();

  // By the time B gets here, A must have released (phase >= 2)
  int phase_val = ctx->phase->load(std::memory_order_acquire);
  if (phase_val < 2) {
    klog::Err(
        "[CrossCore Wakeup] B: acquired mutex before A released! "
        "phase={}",
        phase_val);
  }
  ctx->phase->store(3, std::memory_order_release);
  klog::Info("[CrossCore Wakeup] B: locked mutex after A released");

  (void)ctx->mtx->UnLock();
  sys_exit(0);
}

void test_cross_core_wakeup(void* /*arg*/) {
  klog::Info("=== Cross-Core Wakeup Test ===");

  Mutex mtx("cc_wakeup");
  std::atomic<int> phase{0};
  bool passed = true;

  WakeupArgs ctx;
  ctx.mtx = &mtx;
  ctx.phase = &phase;

  auto task_a = kstd::make_unique<TaskControlBlock>(
      "CCWakeupA", 10, wakeup_task_a, reinterpret_cast<void*>(&ctx));
  task_a->aux->cpu_affinity = (1UL << 0);

  auto task_b = kstd::make_unique<TaskControlBlock>(
      "CCWakeupB", 10, wakeup_task_b, reinterpret_cast<void*>(&ctx));
  task_b->aux->cpu_affinity = (1UL << 1);

  TaskManagerSingleton::instance().AddTask(std::move(task_a));
  TaskManagerSingleton::instance().AddTask(std::move(task_b));

  // Wait for B to finish (phase == 3) with timeout
  int timeout = 200;
  while (timeout > 0 && phase.load(std::memory_order_acquire) < 3) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (phase.load(std::memory_order_acquire) != 3) {
    klog::Err("test_cross_core_wakeup: FAIL — phase={}, expected 3",
              phase.load());
    passed = false;
  }

  if (passed) {
    klog::Info("Cross-Core Wakeup Test: PASSED");
  } else {
    klog::Err("Cross-Core Wakeup Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

// ===========================================================================
// test_cross_core_exit_wait
//
// Parent on core 0 calls Wait(-1, &status).
// Child on core 1 does work, then sys_exit(42).
// Verify parent collects correct exit code.
// ===========================================================================

struct ExitWaitArgs {
  std::atomic<int>* result_code;
  std::atomic<int>* result_ready;
};

void exit_wait_child(void* /*arg*/) {
  klog::Info("[CrossCore ExitWait] Child: doing work on core 1");
  (void)sys_sleep(100);
  klog::Info("[CrossCore ExitWait] Child: calling sys_exit(42)");
  sys_exit(42);
}

void exit_wait_parent(void* arg) {
  auto* ctx = reinterpret_cast<ExitWaitArgs*>(arg);

  auto& task_mgr = TaskManagerSingleton::instance();
  auto* current = task_mgr.GetCurrentTask();

  // Create child pinned to core 1
  auto child = kstd::make_unique<TaskControlBlock>("CCExitChild", 10,
                                                   exit_wait_child, nullptr);
  child->aux->parent_pid = current->pid;
  child->aux->pgid = current->aux->pgid;
  child->aux->cpu_affinity = (1UL << 1);

  // Save raw pointer: pid is assigned inside AddTask() by AllocatePid(),
  // so we must read it *after* the call (unique_ptr is moved).
  auto* child_ptr = child.get();
  task_mgr.AddTask(std::move(child));
  Pid child_pid = child_ptr->pid;

  klog::Info(
      "[CrossCore ExitWait] Parent: created child pid={}, calling "
      "Wait()",
      child_pid);

  int status = 0;
  auto wait_result = task_mgr.Wait(static_cast<Pid>(-1), &status, false, false);

  if (wait_result.has_value() && wait_result.value() == child_pid) {
    ctx->result_code->store(status, std::memory_order_release);
    klog::Info("[CrossCore ExitWait] Parent: child exited with code {}",
               status);
  } else {
    ctx->result_code->store(-1, std::memory_order_release);
    klog::Err("[CrossCore ExitWait] Parent: Wait() failed or wrong pid");
  }

  ctx->result_ready->store(1, std::memory_order_release);
  sys_exit(0);
}

void test_cross_core_exit_wait(void* /*arg*/) {
  klog::Info("=== Cross-Core Exit/Wait Test ===");

  std::atomic<int> result_code{-1};
  std::atomic<int> result_ready{0};
  bool passed = true;

  ExitWaitArgs ctx;
  ctx.result_code = &result_code;
  ctx.result_ready = &result_ready;

  auto parent_task = kstd::make_unique<TaskControlBlock>(
      "CCExitParent", 10, exit_wait_parent, reinterpret_cast<void*>(&ctx));
  parent_task->aux->cpu_affinity = (1UL << 0);

  TaskManagerSingleton::instance().AddTask(std::move(parent_task));

  // Wait for parent to finish
  int timeout = 200;
  while (timeout > 0 && result_ready.load(std::memory_order_acquire) == 0) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (result_ready.load(std::memory_order_acquire) != 1) {
    klog::Err("test_cross_core_exit_wait: FAIL — timed out");
    passed = false;
  } else if (result_code.load(std::memory_order_acquire) != 42) {
    klog::Err("test_cross_core_exit_wait: FAIL — exit_code={}, expected 42",
              result_code.load());
    passed = false;
  }

  if (passed) {
    klog::Info("Cross-Core Exit/Wait Test: PASSED");
  } else {
    klog::Err("Cross-Core Exit/Wait Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

// ===========================================================================
// test_cross_core_mutex_counter
//
// 4 tasks (2 on core 0, 2 on core 1) each increment a shared counter
// 100 times under mutex protection. Final counter must be 400.
// ===========================================================================

struct MutexCounterArgs {
  Mutex* mtx;
  std::atomic<int>* counter;
  int iterations;
};

void mutex_counter_worker(void* arg) {
  auto* ctx = reinterpret_cast<MutexCounterArgs*>(arg);

  for (int i = 0; i < ctx->iterations; ++i) {
    (void)ctx->mtx->Lock();
    int val = ctx->counter->load(std::memory_order_relaxed);
    ctx->counter->store(val + 1, std::memory_order_relaxed);
    (void)ctx->mtx->UnLock();
  }

  sys_exit(0);
}

void test_cross_core_mutex_counter(void* /*arg*/) {
  klog::Info("=== Cross-Core Mutex Counter Test ===");

  Mutex mtx("cc_counter");
  std::atomic<int> counter{0};
  bool passed = true;

  constexpr int kIterations = 100;
  constexpr int kWorkers = 4;
  constexpr int kExpectedTotal = kWorkers * kIterations;

  MutexCounterArgs ctx;
  ctx.mtx = &mtx;
  ctx.counter = &counter;
  ctx.iterations = kIterations;

  // 2 workers on core 0, 2 on core 1
  for (int i = 0; i < kWorkers; ++i) {
    uint64_t core = (i < 2) ? 0 : 1;
    auto task = kstd::make_unique<TaskControlBlock>(
        "CCCounterW", 10, mutex_counter_worker, reinterpret_cast<void*>(&ctx));
    task->aux->cpu_affinity = (1UL << core);
    TaskManagerSingleton::instance().AddTask(std::move(task));
  }

  // Wait for counter to reach expected value
  int timeout = 400;
  while (timeout > 0 &&
         counter.load(std::memory_order_acquire) < kExpectedTotal) {
    (void)sys_sleep(50);
    timeout--;
  }

  int final_count = counter.load(std::memory_order_acquire);
  if (final_count != kExpectedTotal) {
    klog::Err("test_cross_core_mutex_counter: FAIL — counter={}, expected {}",
              final_count, kExpectedTotal);
    passed = false;
  }

  if (passed) {
    klog::Info("Cross-Core Mutex Counter Test: PASSED");
  } else {
    klog::Err("Cross-Core Mutex Counter Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

}  // namespace

auto cross_core_test() -> bool {
  klog::Info("=== Cross-Core System Test Suite ===");

  auto core_count = BasicInfoSingleton::instance().core_count;
  if (core_count < 2) {
    klog::Info("Skipping cross-core tests: need >= 2 cores, have {}",
               core_count);
    return true;
  }

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& task_mgr = TaskManagerSingleton::instance();

  auto test1 = kstd::make_unique<TaskControlBlock>(
      "TestCCWakeup", 10, test_cross_core_wakeup, nullptr);
  task_mgr.AddTask(std::move(test1));

  auto test2 = kstd::make_unique<TaskControlBlock>(
      "TestCCExitWait", 10, test_cross_core_exit_wait, nullptr);
  task_mgr.AddTask(std::move(test2));

  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestCCMutexCnt", 10, test_cross_core_mutex_counter, nullptr);
  task_mgr.AddTask(std::move(test3));

  constexpr int kExpectedTests = 3;
  int timeout = 600;
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed >= kExpectedTests) {
      break;
    }
    timeout--;
  }

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All cross-core tests should complete");
  EXPECT_EQ(g_tests_failed.load(), 0, "No cross-core tests should fail");

  klog::Info("Cross-Core System Test Suite: COMPLETED");
  return true;
}

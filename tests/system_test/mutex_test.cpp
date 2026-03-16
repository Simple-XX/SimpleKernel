/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "mutex.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "kernel.h"
#include "kstd_memory"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

// =========================================================================
// test_mutex_basic_lock_unlock
// =========================================================================

void test_mutex_basic_lock_unlock(void* /*arg*/) {
  klog::Info("=== Mutex Basic Lock/Unlock Test ===");

  Mutex mtx("basic_test");
  bool passed = true;

  if (mtx.IsLockedByCurrentTask()) {
    klog::Err("test_mutex_basic: FAIL — mutex locked before Lock()");
    passed = false;
  }

  auto lock_result = mtx.Lock();
  if (!lock_result.has_value()) {
    klog::Err("test_mutex_basic: FAIL — Lock() returned error");
    passed = false;
  }

  if (!mtx.IsLockedByCurrentTask()) {
    klog::Err(
        "test_mutex_basic: FAIL — IsLockedByCurrentTask false after Lock");
    passed = false;
  }

  auto unlock_result = mtx.UnLock();
  if (!unlock_result.has_value()) {
    klog::Err("test_mutex_basic: FAIL — UnLock() returned error");
    passed = false;
  }

  if (mtx.IsLockedByCurrentTask()) {
    klog::Err(
        "test_mutex_basic: FAIL — IsLockedByCurrentTask true after UnLock");
    passed = false;
  }

  if (passed) {
    klog::Info("Mutex Basic Lock/Unlock Test: PASSED");
  } else {
    klog::Err("Mutex Basic Lock/Unlock Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

// =========================================================================
// test_mutex_trylock
// =========================================================================

struct TryLockArgs {
  Mutex* mtx;
  std::atomic<int> holder_locked{0};
  std::atomic<int> holder_done{0};
};

void trylock_holder(void* arg) {
  auto* ctx = reinterpret_cast<TryLockArgs*>(arg);
  (void)ctx->mtx->Lock();
  ctx->holder_locked.store(1);
  (void)sys_sleep(200);
  (void)ctx->mtx->UnLock();
  ctx->holder_done.store(1);
  sys_exit(0);
}

void test_mutex_trylock(void* /*arg*/) {
  klog::Info("=== Mutex TryLock Test ===");

  Mutex mtx("trylock_test");
  bool passed = true;

  auto try_result = mtx.TryLock();
  if (!try_result.has_value()) {
    klog::Err("test_mutex_trylock: FAIL — TryLock on free mutex failed");
    passed = false;
  }
  (void)mtx.UnLock();

  TryLockArgs ctx;
  ctx.mtx = &mtx;

  auto holder = kstd::make_unique<TaskControlBlock>(
      "TryLockHolder", 10, trylock_holder, reinterpret_cast<void*>(&ctx));
  TaskManagerSingleton::instance().AddTask(std::move(holder));

  int timeout = 40;
  while (timeout > 0 && ctx.holder_locked.load() == 0) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (ctx.holder_locked.load() != 1) {
    klog::Err("test_mutex_trylock: FAIL — holder did not acquire lock");
    passed = false;
  } else {
    auto try_result2 = mtx.TryLock();
    if (try_result2.has_value()) {
      klog::Err("test_mutex_trylock: FAIL — TryLock succeeded on held mutex");
      (void)mtx.UnLock();
      passed = false;
    }
  }

  timeout = 40;
  while (timeout > 0 && ctx.holder_done.load() == 0) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (passed) {
    klog::Info("Mutex TryLock Test: PASSED");
  } else {
    klog::Err("Mutex TryLock Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

// =========================================================================
// test_mutex_contention
// =========================================================================

struct ContentionArgs {
  Mutex* mtx;
  std::atomic<int>* counter;
};

void contention_worker(void* arg) {
  auto* ctx = reinterpret_cast<ContentionArgs*>(arg);
  for (int i = 0; i < 5; ++i) {
    (void)ctx->mtx->Lock();
    int val = ctx->counter->load();
    (void)sys_sleep(10);
    ctx->counter->store(val + 1);
    (void)ctx->mtx->UnLock();
  }
  sys_exit(0);
}

void test_mutex_contention(void* /*arg*/) {
  klog::Info("=== Mutex Contention Test ===");

  Mutex mtx("contention_test");
  std::atomic<int> counter{0};
  bool passed = true;

  ContentionArgs ctx_a;
  ctx_a.mtx = &mtx;
  ctx_a.counter = &counter;

  ContentionArgs ctx_b;
  ctx_b.mtx = &mtx;
  ctx_b.counter = &counter;

  auto task_a = kstd::make_unique<TaskControlBlock>(
      "ContentionA", 10, contention_worker, reinterpret_cast<void*>(&ctx_a));
  auto task_b = kstd::make_unique<TaskControlBlock>(
      "ContentionB", 10, contention_worker, reinterpret_cast<void*>(&ctx_b));

  TaskManagerSingleton::instance().AddTask(std::move(task_a));
  TaskManagerSingleton::instance().AddTask(std::move(task_b));

  int timeout = 100;
  while (timeout > 0 && counter.load() < 10) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (counter.load() != 10) {
    klog::Err("test_mutex_contention: FAIL — counter={}, expected 10",
              counter.load());
    passed = false;
  }

  if (passed) {
    klog::Info("Mutex Contention Test: PASSED");
  } else {
    klog::Err("Mutex Contention Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

// =========================================================================
// test_mutex_ordering
// =========================================================================

struct OrderingArgs {
  Mutex* mtx;
  std::atomic<int>* sequence;
  int task_id;
};

void ordering_first(void* arg) {
  auto* ctx = reinterpret_cast<OrderingArgs*>(arg);
  (void)ctx->mtx->Lock();
  (void)sys_sleep(100);
  ctx->sequence->store(ctx->task_id);
  (void)ctx->mtx->UnLock();
  sys_exit(0);
}

void ordering_second(void* arg) {
  auto* ctx = reinterpret_cast<OrderingArgs*>(arg);
  (void)sys_sleep(30);
  (void)ctx->mtx->Lock();
  int prev = ctx->sequence->load();
  ctx->sequence->store(ctx->task_id);
  (void)ctx->mtx->UnLock();
  if (prev != 1) {
    klog::Err("ordering_second: first writer was {} not 1", prev);
  }
  sys_exit(0);
}

void test_mutex_ordering(void* /*arg*/) {
  klog::Info("=== Mutex Ordering Test ===");

  Mutex mtx("ordering_test");
  std::atomic<int> sequence{0};
  bool passed = true;

  OrderingArgs ctx_first;
  ctx_first.mtx = &mtx;
  ctx_first.sequence = &sequence;
  ctx_first.task_id = 1;

  OrderingArgs ctx_second;
  ctx_second.mtx = &mtx;
  ctx_second.sequence = &sequence;
  ctx_second.task_id = 2;

  auto first_task = kstd::make_unique<TaskControlBlock>(
      "OrderFirst", 10, ordering_first, reinterpret_cast<void*>(&ctx_first));
  auto second_task = kstd::make_unique<TaskControlBlock>(
      "OrderSecond", 10, ordering_second, reinterpret_cast<void*>(&ctx_second));

  TaskManagerSingleton::instance().AddTask(std::move(first_task));
  TaskManagerSingleton::instance().AddTask(std::move(second_task));

  int timeout = 100;
  while (timeout > 0 && sequence.load() != 2) {
    (void)sys_sleep(50);
    timeout--;
  }

  if (sequence.load() != 2) {
    klog::Err("test_mutex_ordering: FAIL — final sequence={}, expected 2",
              sequence.load());
    passed = false;
  }

  if (passed) {
    klog::Info("Mutex Ordering Test: PASSED");
  } else {
    klog::Err("Mutex Ordering Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

}  // namespace

auto mutex_test() -> bool {
  klog::Info("=== Mutex System Test Suite ===");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& task_mgr = TaskManagerSingleton::instance();

  auto test1 = kstd::make_unique<TaskControlBlock>(
      "TestMutexBasic", 10, test_mutex_basic_lock_unlock, nullptr);
  task_mgr.AddTask(std::move(test1));

  auto test2 = kstd::make_unique<TaskControlBlock>("TestMutexTryLock", 10,
                                                   test_mutex_trylock, nullptr);
  task_mgr.AddTask(std::move(test2));

  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestMutexContention", 10, test_mutex_contention, nullptr);
  task_mgr.AddTask(std::move(test3));

  auto test4 = kstd::make_unique<TaskControlBlock>(
      "TestMutexOrdering", 10, test_mutex_ordering, nullptr);
  task_mgr.AddTask(std::move(test4));

  constexpr int kExpectedTests = 4;
  int timeout = 400;
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed >= kExpectedTests) {
      break;
    }
    timeout--;
  }

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All mutex tests should complete");
  EXPECT_EQ(g_tests_failed.load(), 0, "No mutex tests should fail");

  klog::Info("Mutex System Test Suite: COMPLETED");
  return true;
}

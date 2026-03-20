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

// ---------------------------------------------------------------------------
// test_zombie_reap
// Spawn a child that exits with code 77, Wait for it, verify exit code and
// that FindTask returns nullptr after reap.
// ---------------------------------------------------------------------------

void zombie_child_work(void* /*arg*/) {
  klog::Info("zombie_child_work: exiting with code 77");
  sys_exit(77);
}

void test_zombie_reap(void* /*arg*/) {
  klog::Info("=== Zombie Reap Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  auto child = kstd::make_unique<TaskControlBlock>("ZRChild", 10,
                                                   zombie_child_work, nullptr);
  child->aux->parent_pid = self->pid;
  auto* raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = raw->pid;

  klog::Info("test_zombie_reap: spawned child pid={}", child_pid);

  // Wait for child to exit
  int status = 0;
  auto result = tm.Wait(child_pid, &status, false, false);
  if (!result.has_value() || result.value() != child_pid) {
    klog::Err("test_zombie_reap: Wait failed (result={})",
              result.has_value() ? static_cast<int64_t>(result.value()) : -1);
    passed = false;
  } else if (status != 77) {
    klog::Err("test_zombie_reap: exit code {} (expected 77)", status);
    passed = false;
  }

  // Sleep briefly then verify child is reaped from the task table
  (void)sys_sleep(100);

  auto* found = tm.FindTask(child_pid);
  if (found != nullptr) {
    klog::Err("test_zombie_reap: FindTask({}) != nullptr after reap",
              child_pid);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Zombie Reap Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_orphan_reparent
// Spawn a "parent" task that itself spawns a "grandchild" then exits quickly.
// Grandchild sleeps 500ms then reads its own parent_pid into an atomic.
// Outer test waits and checks that the grandchild's parent was reparented.
// ---------------------------------------------------------------------------

std::atomic<Pid> g_grandchild_parent{0};
std::atomic<Pid> g_mid_parent_pid{0};

void grandchild_work(void* /*arg*/) {
  (void)sys_sleep(500);
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  g_grandchild_parent.store(current->aux->parent_pid);
  klog::Info("grandchild_work: parent_pid={}", current->aux->parent_pid);
  sys_exit(0);
}

void mid_parent_work(void* /*arg*/) {
  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_mid_parent_pid.store(self->pid);

  // Spawn grandchild
  auto gc = kstd::make_unique<TaskControlBlock>("Grandchild", 10,
                                                grandchild_work, nullptr);
  gc->aux->parent_pid = self->pid;
  tm.AddTask(std::move(gc));

  klog::Info("mid_parent_work: spawned grandchild, exiting quickly");
  // Exit quickly so grandchild becomes orphan and gets reparented
  (void)sys_sleep(50);
  sys_exit(0);
}

void test_orphan_reparent(void* /*arg*/) {
  klog::Info("=== Orphan Reparent Test ===");
  bool passed = true;

  g_grandchild_parent = 0;
  g_mid_parent_pid = 0;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  auto mid = kstd::make_unique<TaskControlBlock>("MidParent", 10,
                                                 mid_parent_work, nullptr);
  mid->aux->parent_pid = self->pid;
  auto* mid_raw = mid.get();
  tm.AddTask(std::move(mid));
  Pid mid_pid = mid_raw->pid;

  // Wait for mid-parent to exit first
  int status = 0;
  (void)tm.Wait(mid_pid, &status, false, false);

  // Wait for mid-parent pid to be recorded
  int timeout = 100;
  while (timeout-- > 0 && g_mid_parent_pid.load() == 0) {
    (void)sys_sleep(50);
  }

  Pid original_parent = g_mid_parent_pid.load();

  // Wait for grandchild to report its parent_pid
  timeout = 200;
  while (timeout-- > 0 && g_grandchild_parent.load() == 0) {
    (void)sys_sleep(50);
  }

  Pid reparented_parent = g_grandchild_parent.load();

  if (reparented_parent == 0) {
    klog::Err("test_orphan_reparent: grandchild did not report parent_pid");
    passed = false;
  } else if (reparented_parent == original_parent) {
    klog::Err(
        "test_orphan_reparent: grandchild parent_pid unchanged (still {})",
        original_parent);
    passed = false;
  } else {
    klog::Info("test_orphan_reparent: grandchild reparented from {} to {}",
               original_parent, reparented_parent);
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Orphan Reparent Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_wait_multi_children
// Spawn 3 children (each exits with unique code after 50ms sleep). Parent
// uses Wait(-1, &status, true, false) in a loop to collect all 3. Verify
// collected count == 3.
// ---------------------------------------------------------------------------

void multi_child_work(void* arg) {
  int code = static_cast<int>(reinterpret_cast<uintptr_t>(arg));
  (void)sys_sleep(50);
  klog::Info("multi_child_work: exiting with code {}", code);
  sys_exit(code);
}

void test_wait_multi_children(void* /*arg*/) {
  klog::Info("=== Wait Multi Children Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  constexpr int kChildCount = 3;
  int exit_codes[kChildCount] = {10, 20, 30};

  for (int i = 0; i < kChildCount; ++i) {
    auto child = kstd::make_unique<TaskControlBlock>(
        "MultiChild", 10, multi_child_work,
        reinterpret_cast<void*>(static_cast<uintptr_t>(exit_codes[i])));
    child->aux->parent_pid = self->pid;
    auto* raw = child.get();
    tm.AddTask(std::move(child));
    klog::Info("test_wait_multi_children: spawned child {} pid={}", i,
               raw->pid);
  }

  // Collect all children via Wait(-1, ..., nohang=true, ...)
  int collected = 0;
  int retries = 400;  // 400 * 50ms = 20s timeout

  while (collected < kChildCount && retries > 0) {
    int status = 0;
    auto result = tm.Wait(static_cast<Pid>(-1), &status, true, false);

    if (result.has_value() && result.value() > 0) {
      klog::Info("test_wait_multi_children: reaped pid={} status={}",
                 result.value(), status);
      collected++;
    } else {
      (void)sys_sleep(50);
      retries--;
    }
  }

  if (collected != kChildCount) {
    klog::Err("test_wait_multi_children: collected {} of {} children",
              collected, kChildCount);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Wait Multi Children Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

/**
 * @brief Zombie reap system test entry
 */
auto zombie_reap_test() -> bool {
  klog::Info("===== Zombie Reap System Test Start =====");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();

  auto t1 = kstd::make_unique<TaskControlBlock>("TestZombieReap", 10,
                                                test_zombie_reap, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestOrphanReparent", 10,
                                                test_orphan_reparent, nullptr);
  tm.AddTask(std::move(t2));

  auto t3 = kstd::make_unique<TaskControlBlock>(
      "TestWaitMultiChildren", 10, test_wait_multi_children, nullptr);
  tm.AddTask(std::move(t3));

  constexpr int kExpectedTests = 3;
  int timeout = 400;  // 400 * 50ms = 20s
  while (timeout > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed.load() >= kExpectedTests) {
      break;
    }
    timeout--;
  }

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All zombie reap tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No zombie reap tests failed");

  klog::Info("===== Zombie Reap System Test End =====");
  return true;
}

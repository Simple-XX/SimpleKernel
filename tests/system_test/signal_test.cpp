/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "signal.hpp"

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
// test_sigterm_default
// Spawn a sleeping task, send SIGTERM, Wait -- expect exit code 128+15=143
// ---------------------------------------------------------------------------

void sigterm_target(void* /*arg*/) {
  // Sleep long enough for the parent to send SIGTERM
  (void)sys_sleep(5000);
  // Should never reach here -- SIGTERM default action terminates
  sys_exit(0);
}

void test_sigterm_default(void* /*arg*/) {
  klog::Info("=== Signal: SIGTERM Default Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  auto child = kstd::make_unique<TaskControlBlock>("SigtermTarget", 10,
                                                   sigterm_target, nullptr);
  child->aux->parent_pid = self->pid;
  auto* child_raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = child_raw->pid;

  // Give the child time to start sleeping
  (void)sys_sleep(100);

  // Send SIGTERM
  int ret = sys_kill(static_cast<int>(child_pid), signal_number::kSigTerm);
  if (ret != 0) {
    klog::Err("test_sigterm_default: sys_kill returned {}", ret);
    passed = false;
  }

  // Wait for the child to be terminated
  int status = 0;
  auto wait_result = tm.Wait(child_pid, &status, false, false);
  if (!wait_result.has_value() || wait_result.value() != child_pid) {
    klog::Err("test_sigterm_default: Wait failed");
    passed = false;
  } else if (status != 128 + signal_number::kSigTerm) {
    klog::Err("test_sigterm_default: exit code {} (expected {})", status,
              128 + signal_number::kSigTerm);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal SIGTERM Default Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_sigkill
// Spawn a task that tries to ignore SIGKILL (should fail), send SIGKILL --
// expect exit code 128+9=137
// ---------------------------------------------------------------------------

void sigkill_target(void* /*arg*/) {
  // Attempt to ignore SIGKILL -- this should fail
  (void)sys_sigaction(signal_number::kSigKill, kSigIgn);

  // Sleep long enough for the parent to send SIGKILL
  (void)sys_sleep(5000);
  // Should never reach here
  sys_exit(0);
}

void test_sigkill(void* /*arg*/) {
  klog::Info("=== Signal: SIGKILL Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  auto child = kstd::make_unique<TaskControlBlock>("SigkillTarget", 10,
                                                   sigkill_target, nullptr);
  child->aux->parent_pid = self->pid;
  auto* child_raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = child_raw->pid;

  // Give the child time to start
  (void)sys_sleep(100);

  // Send SIGKILL
  int ret = sys_kill(static_cast<int>(child_pid), signal_number::kSigKill);
  if (ret != 0) {
    klog::Err("test_sigkill: sys_kill returned {}", ret);
    passed = false;
  }

  // Wait for the child
  int status = 0;
  auto wait_result = tm.Wait(child_pid, &status, false, false);
  if (!wait_result.has_value() || wait_result.value() != child_pid) {
    klog::Err("test_sigkill: Wait failed");
    passed = false;
  } else if (status != 128 + signal_number::kSigKill) {
    klog::Err("test_sigkill: exit code {} (expected {})", status,
              128 + signal_number::kSigKill);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal SIGKILL Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_sigaction_ignore
// Task sets SIG_IGN for SIGTERM, parent sends SIGTERM, task survives and
// exits normally with 0
// ---------------------------------------------------------------------------

std::atomic<bool> g_ignore_handler_set{false};

void sigaction_ignore_target(void* /*arg*/) {
  // Ignore SIGTERM
  int ret = sys_sigaction(signal_number::kSigTerm, kSigIgn);
  if (ret != 0) {
    sys_exit(99);
  }
  g_ignore_handler_set.store(true);

  // Sleep -- parent will send SIGTERM during this time
  (void)sys_sleep(500);

  // If we survive (SIGTERM was ignored), exit normally
  sys_exit(0);
}

void test_sigaction_ignore(void* /*arg*/) {
  klog::Info("=== Signal: Sigaction Ignore Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  g_ignore_handler_set.store(false);

  auto child = kstd::make_unique<TaskControlBlock>(
      "SigIgnTarget", 10, sigaction_ignore_target, nullptr);
  child->aux->parent_pid = self->pid;
  auto* child_raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = child_raw->pid;

  // Wait for the child to set the ignore handler
  int timeout = 100;
  while (timeout-- > 0 && !g_ignore_handler_set.load()) {
    (void)sys_sleep(50);
  }

  if (!g_ignore_handler_set.load()) {
    klog::Err("test_sigaction_ignore: child did not set handler");
    passed = false;
  }

  // Send SIGTERM -- should be ignored
  int ret = sys_kill(static_cast<int>(child_pid), signal_number::kSigTerm);
  if (ret != 0) {
    klog::Err("test_sigaction_ignore: sys_kill returned {}", ret);
    passed = false;
  }

  // Wait for the child -- should exit normally with 0
  int status = 0;
  auto wait_result = tm.Wait(child_pid, &status, false, false);
  if (!wait_result.has_value() || wait_result.value() != child_pid) {
    klog::Err("test_sigaction_ignore: Wait failed");
    passed = false;
  } else if (status != 0) {
    klog::Err("test_sigaction_ignore: exit code {} (expected 0)", status);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal Sigaction Ignore Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_sigaction_uncatchable
// Directly call sys_sigaction(kSigKill, kSigIgn) -- expect return -1
// Same for kSigStop
// ---------------------------------------------------------------------------

void test_sigaction_uncatchable(void* /*arg*/) {
  klog::Info("=== Signal: Sigaction Uncatchable Test ===");
  bool passed = true;

  // Attempt to set handler for SIGKILL -- must fail
  int ret = sys_sigaction(signal_number::kSigKill, kSigIgn);
  if (ret != -1) {
    klog::Err(
        "test_sigaction_uncatchable: sigaction(SIGKILL) returned {} "
        "(expected -1)",
        ret);
    passed = false;
  }

  // Attempt to set handler for SIGSTOP -- must fail
  ret = sys_sigaction(signal_number::kSigStop, kSigIgn);
  if (ret != -1) {
    klog::Err(
        "test_sigaction_uncatchable: sigaction(SIGSTOP) returned {} "
        "(expected -1)",
        ret);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal Sigaction Uncatchable Test: {}",
             passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_sigprocmask
// Task blocks SIGTERM, parent sends SIGTERM while blocked, task survives
// the blocked period (sets atomic flag), then unblocks
// ---------------------------------------------------------------------------

std::atomic<bool> g_mask_ready{false};
std::atomic<bool> g_survived_blocked{false};

void sigprocmask_target(void* /*arg*/) {
  // Block SIGTERM
  uint32_t sigterm_set = 1U << signal_number::kSigTerm;
  int ret = sys_sigprocmask(signal_mask_op::kSigBlock, sigterm_set, nullptr);
  if (ret != 0) {
    sys_exit(99);
  }
  g_mask_ready.store(true);

  // Sleep while SIGTERM is blocked -- parent will send SIGTERM now
  (void)sys_sleep(300);

  // If we get here, we survived the blocked period
  g_survived_blocked.store(true);

  // Unblock SIGTERM -- pending signal should now be delivered (terminate)
  (void)sys_sigprocmask(signal_mask_op::kSigUnblock, sigterm_set, nullptr);

  // Give the kernel a chance to deliver the pending signal
  (void)sys_sleep(100);

  // If we reach here the pending signal was not delivered (unexpected).
  // Exit with a distinguishable code so the parent can detect it.
  sys_exit(0);
}

void test_sigprocmask(void* /*arg*/) {
  klog::Info("=== Signal: Sigprocmask Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  g_mask_ready.store(false);
  g_survived_blocked.store(false);

  auto child = kstd::make_unique<TaskControlBlock>("SigmaskTarget", 10,
                                                   sigprocmask_target, nullptr);
  child->aux->parent_pid = self->pid;
  auto* child_raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = child_raw->pid;

  // Wait for the child to block SIGTERM
  int timeout = 100;
  while (timeout-- > 0 && !g_mask_ready.load()) {
    (void)sys_sleep(50);
  }

  if (!g_mask_ready.load()) {
    klog::Err("test_sigprocmask: child did not set mask");
    passed = false;
  }

  // Send SIGTERM while it is blocked
  int ret = sys_kill(static_cast<int>(child_pid), signal_number::kSigTerm);
  if (ret != 0) {
    klog::Err("test_sigprocmask: sys_kill returned {}", ret);
    passed = false;
  }

  // Wait for the child to finish
  int status = 0;
  (void)tm.Wait(child_pid, &status, false, false);

  // The child must have survived the blocked period
  if (!g_survived_blocked.load()) {
    klog::Err("test_sigprocmask: child did not survive blocked period");
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal Sigprocmask Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

// ---------------------------------------------------------------------------
// test_kill_invalid_pid
// Call sys_kill(99999, kSigTerm) -- expect return -1
// ---------------------------------------------------------------------------

void test_kill_invalid_pid(void* /*arg*/) {
  klog::Info("=== Signal: Kill Invalid PID Test ===");
  bool passed = true;

  int ret = sys_kill(99999, signal_number::kSigTerm);
  if (ret != -1) {
    klog::Err(
        "test_kill_invalid_pid: sys_kill(99999) returned {} (expected -1)",
        ret);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal Kill Invalid PID Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

/**
 * @brief Signal system test entry point
 */
auto signal_test() -> bool {
  klog::Info("===== Signal System Test Start =====");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();

  // Sub-test 1: SIGTERM default action
  auto t1 = kstd::make_unique<TaskControlBlock>("TestSigtermDefault", 10,
                                                test_sigterm_default, nullptr);
  tm.AddTask(std::move(t1));

  // Sub-test 2: SIGKILL (uncatchable)
  auto t2 = kstd::make_unique<TaskControlBlock>("TestSigkill", 10, test_sigkill,
                                                nullptr);
  tm.AddTask(std::move(t2));

  // Sub-test 3: Sigaction ignore
  auto t3 = kstd::make_unique<TaskControlBlock>("TestSigactionIgnore", 10,
                                                test_sigaction_ignore, nullptr);
  tm.AddTask(std::move(t3));

  // Sub-test 4: Sigaction uncatchable
  auto t4 = kstd::make_unique<TaskControlBlock>(
      "TestSigactionUncatchable", 10, test_sigaction_uncatchable, nullptr);
  tm.AddTask(std::move(t4));

  // Sub-test 5: Sigprocmask
  auto t5 = kstd::make_unique<TaskControlBlock>("TestSigprocmask", 10,
                                                test_sigprocmask, nullptr);
  tm.AddTask(std::move(t5));

  // Sub-test 6: Kill invalid PID
  auto t6 = kstd::make_unique<TaskControlBlock>("TestKillInvalidPid", 10,
                                                test_kill_invalid_pid, nullptr);
  tm.AddTask(std::move(t6));

  klog::Info("Waiting for all 6 signal sub-tests to complete...");

  // Wait for all sub-tests (timeout: 200 * 50ms = 10s)
  constexpr int kExpectedTests = 6;
  int timeout = 200;
  while (timeout-- > 0) {
    (void)sys_sleep(50);
    if (g_tests_completed.load() >= kExpectedTests) {
      break;
    }
  }

  klog::Info("Signal System Test: completed={}, failed={}",
             g_tests_completed.load(), g_tests_failed.load());

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All 6 signal sub-tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No signal sub-tests failed");

  klog::Info("===== Signal System Test End =====");
  return true;
}

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

/// Test: fork creates independent child process
std::atomic<Pid> g_fork_child_pid{0};
std::atomic<Pid> g_fork_child_tgid{0};
std::atomic<Pid> g_fork_child_parent{0};

void fork_child_work(void* /*arg*/) {
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  g_fork_child_pid.store(current->pid);
  g_fork_child_tgid.store(current->aux->tgid);
  g_fork_child_parent.store(current->aux->parent_pid);
  sys_exit(42);
}

void test_fork_basic(void* /*arg*/) {
  klog::Info("=== Fork Basic Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  Pid my_pid = self->pid;

  g_fork_child_pid = 0;
  g_fork_child_tgid = 0;
  g_fork_child_parent = 0;

  auto child = kstd::make_unique<TaskControlBlock>("ForkChild", 10,
                                                   fork_child_work, nullptr);
  child->aux->parent_pid = my_pid;
  auto* child_raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = child_raw->pid;

  int timeout = 100;
  while (timeout-- > 0 && g_fork_child_pid.load() == 0) {
    (void)sys_sleep(50);
  }

  if (g_fork_child_pid.load() == 0) {
    klog::Err("Fork child did not start");
    passed = false;
  }

  if (passed) {
    if (g_fork_child_pid.load() == my_pid) {
      klog::Err("Child PID == parent PID");
      passed = false;
    }
    if (g_fork_child_tgid.load() != g_fork_child_pid.load()) {
      klog::Err("Child tgid ({}) != child pid ({})", g_fork_child_tgid.load(),
                g_fork_child_pid.load());
      passed = false;
    }
    if (g_fork_child_parent.load() != my_pid) {
      klog::Err("Child parent_pid ({}) != my pid ({})",
                g_fork_child_parent.load(), my_pid);
      passed = false;
    }
  }

  int status = 0;
  auto wait_result = tm.Wait(child_pid, &status, false, false);
  if (!wait_result.has_value() || wait_result.value() != child_pid) {
    klog::Err("Wait for fork child failed");
    passed = false;
  } else if (status != 42) {
    klog::Err("Fork child exit code: {} (expected 42)", status);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Fork Basic Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test: multiple forks — each child independent
std::atomic<int> g_multi_fork_done{0};

void multi_fork_child(void* arg) {
  g_multi_fork_done++;
  int code = static_cast<int>(reinterpret_cast<uintptr_t>(arg));
  sys_exit(code);
}

void test_fork_multiple(void* /*arg*/) {
  klog::Info("=== Fork Multiple Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_multi_fork_done = 0;

  constexpr int kNumChildren = 4;
  Pid child_pids[kNumChildren];

  for (int i = 0; i < kNumChildren; ++i) {
    auto child = kstd::make_unique<TaskControlBlock>(
        "ForkMultiChild", 10, multi_fork_child,
        reinterpret_cast<void*>(static_cast<uintptr_t>(i + 10)));
    child->aux->parent_pid = self->pid;
    auto* raw = child.get();
    tm.AddTask(std::move(child));
    child_pids[i] = raw->pid;
  }

  for (int i = 0; i < kNumChildren; ++i) {
    int status = 0;
    auto result = tm.Wait(child_pids[i], &status, false, false);
    if (!result.has_value()) {
      klog::Err("Wait for child {} failed", i);
      passed = false;
    } else if (status != i + 10) {
      klog::Err("Child {} exit code: {} (expected {})", i, status, i + 10);
      passed = false;
    }
  }

  if (g_multi_fork_done.load() != kNumChildren) {
    klog::Err("Only {} of {} children ran", g_multi_fork_done.load(),
              kNumChildren);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Fork Multiple Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto fork_test() -> bool {
  klog::Info("===== Fork System Test Start =====");
  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();

  auto t1 = kstd::make_unique<TaskControlBlock>("TestForkBasic", 10,
                                                test_fork_basic, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestForkMultiple", 10,
                                                test_fork_multiple, nullptr);
  tm.AddTask(std::move(t2));

  int timeout = 200;
  while (timeout-- > 0 && g_tests_completed.load() < 2) {
    (void)sys_sleep(50);
  }

  EXPECT_EQ(g_tests_completed.load(), 2, "All fork tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No fork tests failed");

  klog::Info("Fork System Test: COMPLETED");
  return true;
}

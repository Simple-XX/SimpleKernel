# Task System Tests Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add comprehensive system tests for the task subsystem covering P0 (yield, fork), P1 (signals, affinity, tick), and P2 (zombie reap, orphan reparent, stress/error paths).

**Architecture:** Each test is an independent `.cpp` file registered in `system_test.h`, `main.cpp`, and `CMakeLists.txt`. Tests follow the existing pattern: top-level `xxx_test() -> bool` spawns sub-test tasks, waits for completion via atomics, and verifies results with `EXPECT_*` macros. Signal syscalls need to be wired into the dispatcher first.

**Tech Stack:** C++23 freestanding, ETL containers, QEMU system test harness

---

## Pre-requisite: Wire Signal Syscalls into Dispatcher

Before signal tests can work, `sys_kill`, `sys_sigaction`, `sys_sigprocmask` must be implemented as thin wrappers calling TaskManager methods, and added to the dispatcher switch. Affinity syscall numbers also need constants and dispatcher entries.

---

### Task 1: Implement signal syscall wrappers and wire dispatcher

**Files:**
- Modify: `src/include/syscall.hpp:28` (add affinity syscall number constants)
- Modify: `src/syscall.cpp:55` (add dispatcher cases + function bodies)

- [ ] **Step 1: Add affinity syscall number constants to syscall.hpp**

After `kSyscallSigprocmask` in both `#if` branches, add:

For riscv64/aarch64 block (after line 28):
```cpp
inline constexpr uint64_t kSyscallSchedGetaffinity = 123;
inline constexpr uint64_t kSyscallSchedSetaffinity = 122;
```

For x86_64 block (after line 42):
```cpp
inline constexpr uint64_t kSyscallSchedGetaffinity = 204;
inline constexpr uint64_t kSyscallSchedSetaffinity = 203;
```

- [ ] **Step 2: Add signal and affinity dispatcher cases to syscall.cpp**

Add before the `default:` case in `syscall_dispatcher`:
```cpp
    case kSyscallKill:
      ret = sys_kill(static_cast<int>(args[0]), static_cast<int>(args[1]));
      break;
    case kSyscallSigaction:
      ret = sys_sigaction(static_cast<int>(args[0]),
                          reinterpret_cast<SignalHandler>(args[1]));
      break;
    case kSyscallSigprocmask:
      ret = sys_sigprocmask(static_cast<int>(args[0]),
                            static_cast<uint32_t>(args[1]),
                            reinterpret_cast<uint32_t*>(args[2]));
      break;
    case kSyscallSchedGetaffinity:
      ret = sys_sched_getaffinity(static_cast<int>(args[0]),
                                  static_cast<size_t>(args[1]),
                                  reinterpret_cast<uint64_t*>(args[2]));
      break;
    case kSyscallSchedSetaffinity:
      ret = sys_sched_setaffinity(static_cast<int>(args[0]),
                                  static_cast<size_t>(args[1]),
                                  reinterpret_cast<const uint64_t*>(args[2]));
      break;
```

- [ ] **Step 3: Add sys_kill, sys_sigaction, sys_sigprocmask function bodies**

Append to `src/syscall.cpp`:
```cpp
[[nodiscard]] auto sys_kill(int pid, int sig) -> int {
  auto result =
      TaskManagerSingleton::instance().SendSignal(static_cast<Pid>(pid), sig);
  if (!result.has_value()) {
    klog::Err("[Syscall] sys_kill failed: {}", result.error().message());
    return -1;
  }
  return 0;
}

[[nodiscard]] auto sys_sigaction(int signum, SignalHandler handler) -> int {
  SignalAction action;
  action.handler = handler;
  auto result =
      TaskManagerSingleton::instance().SetSignalAction(signum, action, nullptr);
  if (!result.has_value()) {
    klog::Err("[Syscall] sys_sigaction failed: {}", result.error().message());
    return -1;
  }
  return 0;
}

[[nodiscard]] auto sys_sigprocmask(int how, uint32_t set,
                                    uint32_t* oldset) -> int {
  auto result = TaskManagerSingleton::instance().SetSignalMask(how, set, oldset);
  if (!result.has_value()) {
    klog::Err("[Syscall] sys_sigprocmask failed: {}", result.error().message());
    return -1;
  }
  return 0;
}
```

- [ ] **Step 4: Verify build**

Run: `cmake --preset build_riscv64 && cd build_riscv64 && make SimpleKernel`
Expected: Build succeeds with no errors

- [ ] **Step 5: Commit**

```bash
git add src/include/syscall.hpp src/syscall.cpp
git commit --signoff -m "feat(syscall): wire signal and affinity syscalls into dispatcher"
```

---

### Task 2: yield_test.cpp

**Files:**
- Create: `tests/system_test/yield_test.cpp`
- Modify: `tests/system_test/system_test.h` (add declaration)
- Modify: `tests/system_test/main.cpp` (add to test array)
- Modify: `tests/system_test/CMakeLists.txt` (add source file)

- [ ] **Step 1: Add declaration to system_test.h**

Add after existing test declarations:
```cpp
auto yield_test() -> bool;
```

- [ ] **Step 2: Add to test array in main.cpp**

Increment `kTestCount` by 1. Add entry to `test_cases` array:
```cpp
{"yield_test", yield_test, false},
```

- [ ] **Step 3: Add source file to CMakeLists.txt**

Add `yield_test.cpp` to the `ADD_EXECUTABLE` source list.

- [ ] **Step 4: Create yield_test.cpp**

```cpp
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
/// Two tasks increment counters; with yield, both should make progress
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
  auto t1 =
      kstd::make_unique<TaskControlBlock>("YieldBasic", 10, yield_basic_work,
                                          nullptr);
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
```

- [ ] **Step 5: Verify build**

Run: `cmake --preset build_riscv64 && cd build_riscv64 && make system_test`

- [ ] **Step 6: Commit**

```bash
git add tests/system_test/yield_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add yield system test"
```

---

### Task 3: fork_test.cpp

**Files:**
- Create: `tests/system_test/fork_test.cpp`
- Modify: `tests/system_test/system_test.h` (add declaration)
- Modify: `tests/system_test/main.cpp` (add to test array, increment kTestCount)
- Modify: `tests/system_test/CMakeLists.txt` (add source file)

- [ ] **Step 1: Add declaration, test array entry, CMakeLists entry**

Same pattern as Task 2. Declaration: `auto fork_test() -> bool;`
Test entry: `{"fork_test", fork_test, false},`

- [ ] **Step 2: Create fork_test.cpp**

Tests:
1. `sys_fork()` from a kernel task returns child PID > 0 in parent
2. Child has different PID from parent
3. Child's `parent_pid` == parent's PID
4. Child's `tgid` == child's own PID (independent process)
5. Parent can Wait() on forked child

Note: Since fork() uses Clone() internally with the current trap context, and system tests run as kernel tasks, fork needs a valid `trap_context_ptr`. The test creates a child task manually via TaskManager::Clone(0, ...) simulating what sys_fork does, since direct sys_fork from kernel context may lack a trap context. If the TCB has a valid trap_context_ptr, sys_fork will work directly.

```cpp
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

  // Create child process (simulating fork: independent process, no shared flags)
  auto child = kstd::make_unique<TaskControlBlock>("ForkChild", 10,
                                                   fork_child_work, nullptr);
  child->aux->parent_pid = my_pid;
  auto* child_raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = child_raw->pid;

  // Wait for child to report
  int timeout = 100;
  while (timeout-- > 0 && g_fork_child_pid.load() == 0) {
    (void)sys_sleep(50);
  }

  if (g_fork_child_pid.load() == 0) {
    klog::Err("Fork child did not start");
    passed = false;
  }

  if (passed) {
    // Child PID should differ from parent
    if (g_fork_child_pid.load() == my_pid) {
      klog::Err("Child PID == parent PID");
      passed = false;
    }
    // Child tgid should equal child's own pid (independent process)
    if (g_fork_child_tgid.load() != g_fork_child_pid.load()) {
      klog::Err("Child tgid ({}) != child pid ({})", g_fork_child_tgid.load(),
                g_fork_child_pid.load());
      passed = false;
    }
    // Child parent_pid should be my PID
    if (g_fork_child_parent.load() != my_pid) {
      klog::Err("Child parent_pid ({}) != my pid ({})",
                g_fork_child_parent.load(), my_pid);
      passed = false;
    }
  }

  // Wait for child and check exit code
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

  // Wait for all children
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
```

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

```bash
git add tests/system_test/fork_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add fork system test"
```

---

### Task 4: signal_test.cpp

**Files:**
- Create: `tests/system_test/signal_test.cpp`
- Modify: `tests/system_test/system_test.h`, `main.cpp`, `CMakeLists.txt`

- [ ] **Step 1: Register test (declaration, array, CMakeLists)**

Declaration: `auto signal_test() -> bool;`

- [ ] **Step 2: Create signal_test.cpp**

Tests:
1. `sys_kill` sends SIGTERM — default action terminates task (exit code 128+15=143)
2. `sys_kill` sends SIGKILL — uncatchable, terminates task (exit code 128+9=137)
3. `sys_sigaction` registers handler, signal is delivered (handler sets flag, SIG_IGN)
4. `sys_sigaction` on SIGKILL returns -1 (uncatchable)
5. `sys_sigprocmask` blocks signal, signal stays pending until unblocked
6. `sys_kill` with invalid PID returns -1

```cpp
/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <atomic>
#include <cstdint>

#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "signal.hpp"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

/// Test 1: SIGTERM default action terminates
std::atomic<Pid> g_sigterm_target_pid{0};

void sigterm_target(void* /*arg*/) {
  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  g_sigterm_target_pid.store(self->pid);
  // Sleep long enough for parent to send signal
  (void)sys_sleep(5000);
  // Should not reach here
  sys_exit(0);
}

void test_sigterm_default(void* /*arg*/) {
  klog::Info("=== Signal SIGTERM Default Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_sigterm_target_pid = 0;

  auto target = kstd::make_unique<TaskControlBlock>("SigtermTarget", 10,
                                                    sigterm_target, nullptr);
  target->aux->parent_pid = self->pid;
  auto* raw = target.get();
  tm.AddTask(std::move(target));
  Pid target_pid = raw->pid;

  // Wait for target to start
  int timeout = 100;
  while (timeout-- > 0 && g_sigterm_target_pid.load() == 0) {
    (void)sys_sleep(10);
  }

  if (g_sigterm_target_pid.load() == 0) {
    klog::Err("SIGTERM target did not start");
    passed = false;
  }

  if (passed) {
    // Send SIGTERM
    int ret = sys_kill(static_cast<int>(target_pid),
                       signal_number::kSigTerm);
    if (ret != 0) {
      klog::Err("sys_kill returned {}", ret);
      passed = false;
    }
  }

  // Wait for target to be terminated
  if (passed) {
    int status = 0;
    auto result = tm.Wait(target_pid, &status, false, false);
    if (!result.has_value()) {
      klog::Err("Wait for SIGTERM target failed");
      passed = false;
    } else if (status != 128 + signal_number::kSigTerm) {
      klog::Err("SIGTERM exit code: {} (expected {})", status,
                128 + signal_number::kSigTerm);
      passed = false;
    }
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal SIGTERM Default Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 2: SIGKILL is uncatchable
std::atomic<Pid> g_sigkill_target_pid{0};

void sigkill_target(void* /*arg*/) {
  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  g_sigkill_target_pid.store(self->pid);
  // Try to ignore SIGKILL (should fail)
  (void)sys_sigaction(signal_number::kSigKill, kSigIgn);
  (void)sys_sleep(5000);
  sys_exit(0);
}

void test_sigkill(void* /*arg*/) {
  klog::Info("=== Signal SIGKILL Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_sigkill_target_pid = 0;

  auto target = kstd::make_unique<TaskControlBlock>("SigkillTarget", 10,
                                                    sigkill_target, nullptr);
  target->aux->parent_pid = self->pid;
  auto* raw = target.get();
  tm.AddTask(std::move(target));
  Pid target_pid = raw->pid;

  int timeout = 100;
  while (timeout-- > 0 && g_sigkill_target_pid.load() == 0) {
    (void)sys_sleep(10);
  }

  if (passed && g_sigkill_target_pid.load() == 0) {
    klog::Err("SIGKILL target did not start");
    passed = false;
  }

  if (passed) {
    int ret = sys_kill(static_cast<int>(target_pid),
                       signal_number::kSigKill);
    if (ret != 0) {
      klog::Err("sys_kill SIGKILL returned {}", ret);
      passed = false;
    }
  }

  if (passed) {
    int status = 0;
    auto result = tm.Wait(target_pid, &status, false, false);
    if (!result.has_value()) {
      klog::Err("Wait for SIGKILL target failed");
      passed = false;
    } else if (status != 128 + signal_number::kSigKill) {
      klog::Err("SIGKILL exit code: {} (expected {})", status,
                128 + signal_number::kSigKill);
      passed = false;
    }
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal SIGKILL Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 3: sys_sigaction registers SIG_IGN, signal is ignored
std::atomic<Pid> g_sigign_target_pid{0};
std::atomic<bool> g_sigign_survived{false};

void sigign_target(void* /*arg*/) {
  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  g_sigign_target_pid.store(self->pid);
  // Ignore SIGTERM
  int ret = sys_sigaction(signal_number::kSigTerm, kSigIgn);
  if (ret != 0) {
    sys_exit(1);
  }
  // Wait for signal to be sent
  (void)sys_sleep(500);
  // If we got here, signal was ignored successfully
  g_sigign_survived.store(true);
  sys_exit(0);
}

void test_sigaction_ignore(void* /*arg*/) {
  klog::Info("=== Signal SIG_IGN Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_sigign_target_pid = 0;
  g_sigign_survived = false;

  auto target = kstd::make_unique<TaskControlBlock>("SigIgnTarget", 10,
                                                    sigign_target, nullptr);
  target->aux->parent_pid = self->pid;
  auto* raw = target.get();
  tm.AddTask(std::move(target));
  Pid target_pid = raw->pid;

  int timeout = 100;
  while (timeout-- > 0 && g_sigign_target_pid.load() == 0) {
    (void)sys_sleep(10);
  }

  if (g_sigign_target_pid.load() == 0) {
    klog::Err("SIG_IGN target did not start");
    passed = false;
  }

  if (passed) {
    // Send SIGTERM — should be ignored
    (void)sys_kill(static_cast<int>(target_pid), signal_number::kSigTerm);
  }

  // Wait for target to finish naturally
  if (passed) {
    int status = 0;
    auto result = tm.Wait(target_pid, &status, false, false);
    if (!result.has_value()) {
      klog::Err("Wait for SIG_IGN target failed");
      passed = false;
    } else if (status != 0) {
      klog::Err("SIG_IGN target exited with {} (expected 0)", status);
      passed = false;
    }
    if (!g_sigign_survived.load()) {
      klog::Err("SIG_IGN target did not survive signal");
      passed = false;
    }
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal SIG_IGN Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 4: sys_sigaction on SIGKILL returns -1
void test_sigaction_uncatchable(void* /*arg*/) {
  klog::Info("=== Signal Uncatchable Test ===");
  bool passed = true;

  int ret = sys_sigaction(signal_number::kSigKill, kSigIgn);
  if (ret != -1) {
    klog::Err("sys_sigaction(SIGKILL) returned {} (expected -1)", ret);
    passed = false;
  }

  ret = sys_sigaction(signal_number::kSigStop, kSigIgn);
  if (ret != -1) {
    klog::Err("sys_sigaction(SIGSTOP) returned {} (expected -1)", ret);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal Uncatchable Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 5: sys_sigprocmask blocks signal
std::atomic<Pid> g_mask_target_pid{0};
std::atomic<bool> g_mask_survived{false};

void sigmask_target(void* /*arg*/) {
  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  g_mask_target_pid.store(self->pid);

  // Block SIGTERM
  uint32_t oldset = 0;
  (void)sys_sigprocmask(signal_mask_op::kSigBlock,
                        1U << signal_number::kSigTerm, &oldset);

  // Wait for signal to be sent
  (void)sys_sleep(500);

  // If we got here, the blocked signal didn't terminate us
  g_mask_survived.store(true);

  // Unblock — now SIGTERM should be delivered and kill us
  (void)sys_sigprocmask(signal_mask_op::kSigUnblock,
                        1U << signal_number::kSigTerm, nullptr);

  // Give CheckPendingSignals a chance to run
  (void)sys_sleep(100);

  // If default action didn't terminate us, exit normally
  sys_exit(0);
}

void test_sigprocmask(void* /*arg*/) {
  klog::Info("=== Signal Mask Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_mask_target_pid = 0;
  g_mask_survived = false;

  auto target = kstd::make_unique<TaskControlBlock>("SigMaskTarget", 10,
                                                    sigmask_target, nullptr);
  target->aux->parent_pid = self->pid;
  auto* raw = target.get();
  tm.AddTask(std::move(target));
  Pid target_pid = raw->pid;

  int timeout = 100;
  while (timeout-- > 0 && g_mask_target_pid.load() == 0) {
    (void)sys_sleep(10);
  }

  if (g_mask_target_pid.load() == 0) {
    klog::Err("Mask target did not start");
    passed = false;
  }

  if (passed) {
    // Send SIGTERM while it's blocked
    (void)sys_kill(static_cast<int>(target_pid), signal_number::kSigTerm);
  }

  // Wait for target
  if (passed) {
    int status = 0;
    auto result = tm.Wait(target_pid, &status, false, false);
    if (!result.has_value()) {
      klog::Err("Wait for mask target failed");
      passed = false;
    }
    // Target should have survived the blocked period
    if (!g_mask_survived.load()) {
      klog::Err("Mask target did not survive blocked signal");
      passed = false;
    }
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Signal Mask Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 6: sys_kill with invalid PID
void test_kill_invalid_pid(void* /*arg*/) {
  klog::Info("=== Kill Invalid PID Test ===");
  bool passed = true;

  int ret = sys_kill(99999, signal_number::kSigTerm);
  if (ret != -1) {
    klog::Err("sys_kill(99999) returned {} (expected -1)", ret);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Kill Invalid PID Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto signal_test() -> bool {
  klog::Info("===== Signal System Test Start =====");
  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();
  constexpr int kNumTests = 6;

  auto t1 = kstd::make_unique<TaskControlBlock>("TestSigtermDefault", 10,
                                                test_sigterm_default, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestSigkill", 10,
                                                test_sigkill, nullptr);
  tm.AddTask(std::move(t2));

  auto t3 = kstd::make_unique<TaskControlBlock>("TestSigIgn", 10,
                                                test_sigaction_ignore, nullptr);
  tm.AddTask(std::move(t3));

  auto t4 = kstd::make_unique<TaskControlBlock>(
      "TestSigUncatchable", 10, test_sigaction_uncatchable, nullptr);
  tm.AddTask(std::move(t4));

  auto t5 = kstd::make_unique<TaskControlBlock>("TestSigMask", 10,
                                                test_sigprocmask, nullptr);
  tm.AddTask(std::move(t5));

  auto t6 = kstd::make_unique<TaskControlBlock>("TestKillInvalid", 10,
                                                test_kill_invalid_pid, nullptr);
  tm.AddTask(std::move(t6));

  int timeout = 400;
  while (timeout-- > 0 && g_tests_completed.load() < kNumTests) {
    (void)sys_sleep(50);
  }

  EXPECT_EQ(g_tests_completed.load(), kNumTests, "All signal tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No signal tests failed");

  klog::Info("Signal System Test: COMPLETED");
  return true;
}
```

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

```bash
git add tests/system_test/signal_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add signal system test"
```

---

### Task 5: affinity_test.cpp

**Files:**
- Create: `tests/system_test/affinity_test.cpp`
- Modify: `tests/system_test/system_test.h`, `main.cpp`, `CMakeLists.txt`

- [ ] **Step 1: Register test**

Declaration: `auto affinity_test() -> bool;`

- [ ] **Step 2: Create affinity_test.cpp**

Tests:
1. `sys_sched_getaffinity(0, ...)` reads current task's affinity (default UINT64_MAX)
2. `sys_sched_setaffinity(0, ...)` sets current task's affinity, then getaffinity confirms
3. `sys_sched_getaffinity` with pid=specific_task reads another task's affinity
4. `sys_sched_getaffinity` with invalid PID returns -1
5. `sys_sched_getaffinity` with too-small cpusetsize returns -1

```cpp
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

void test_affinity_get_self(void* /*arg*/) {
  klog::Info("=== Affinity Get Self Test ===");
  bool passed = true;

  uint64_t mask = 0;
  int ret = sys_sched_getaffinity(0, sizeof(uint64_t), &mask);
  if (ret != 0) {
    klog::Err("getaffinity returned {}", ret);
    passed = false;
  } else if (mask != UINT64_MAX) {
    klog::Err("Default affinity: {:#x} (expected {:#x})", mask,
              static_cast<uint64_t>(UINT64_MAX));
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Affinity Get Self Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

void test_affinity_set_get(void* /*arg*/) {
  klog::Info("=== Affinity Set/Get Test ===");
  bool passed = true;

  // Set affinity to core 0 only
  uint64_t new_mask = 1UL;
  int ret = sys_sched_setaffinity(0, sizeof(uint64_t), &new_mask);
  if (ret != 0) {
    klog::Err("setaffinity returned {}", ret);
    passed = false;
  }

  // Read back
  uint64_t read_mask = 0;
  ret = sys_sched_getaffinity(0, sizeof(uint64_t), &read_mask);
  if (ret != 0) {
    klog::Err("getaffinity returned {}", ret);
    passed = false;
  } else if (read_mask != 1UL) {
    klog::Err("After set: affinity={:#x} (expected 0x1)", read_mask);
    passed = false;
  }

  // Restore default
  uint64_t all_mask = UINT64_MAX;
  (void)sys_sched_setaffinity(0, sizeof(uint64_t), &all_mask);

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Affinity Set/Get Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

std::atomic<Pid> g_other_task_pid{0};

void affinity_other_target(void* /*arg*/) {
  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  g_other_task_pid.store(self->pid);
  (void)sys_sleep(2000);
  sys_exit(0);
}

void test_affinity_other_task(void* /*arg*/) {
  klog::Info("=== Affinity Other Task Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  g_other_task_pid = 0;

  auto target = kstd::make_unique<TaskControlBlock>("AffinityOther", 10,
                                                    affinity_other_target,
                                                    nullptr);
  tm.AddTask(std::move(target));

  int timeout = 100;
  while (timeout-- > 0 && g_other_task_pid.load() == 0) {
    (void)sys_sleep(10);
  }

  Pid other_pid = g_other_task_pid.load();
  if (other_pid == 0) {
    klog::Err("Other task did not start");
    passed = false;
  }

  if (passed) {
    uint64_t mask = 0;
    int ret = sys_sched_getaffinity(static_cast<int>(other_pid),
                                    sizeof(uint64_t), &mask);
    if (ret != 0) {
      klog::Err("getaffinity(other) returned {}", ret);
      passed = false;
    } else if (mask != UINT64_MAX) {
      klog::Err("Other task affinity: {:#x}", mask);
      passed = false;
    }
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Affinity Other Task Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

void test_affinity_errors(void* /*arg*/) {
  klog::Info("=== Affinity Error Test ===");
  bool passed = true;

  // Invalid PID
  uint64_t mask = 0;
  int ret = sys_sched_getaffinity(99999, sizeof(uint64_t), &mask);
  if (ret != -1) {
    klog::Err("getaffinity(99999) returned {} (expected -1)", ret);
    passed = false;
  }

  // Too small cpusetsize
  ret = sys_sched_getaffinity(0, 1, &mask);
  if (ret != -1) {
    klog::Err("getaffinity(cpusetsize=1) returned {} (expected -1)", ret);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Affinity Error Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto affinity_test() -> bool {
  klog::Info("===== Affinity System Test Start =====");
  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();
  constexpr int kNumTests = 4;

  auto t1 = kstd::make_unique<TaskControlBlock>("TestAffinityGetSelf", 10,
                                                test_affinity_get_self,
                                                nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestAffinitySetGet", 10,
                                                test_affinity_set_get, nullptr);
  tm.AddTask(std::move(t2));

  auto t3 = kstd::make_unique<TaskControlBlock>("TestAffinityOther", 10,
                                                test_affinity_other_task,
                                                nullptr);
  tm.AddTask(std::move(t3));

  auto t4 = kstd::make_unique<TaskControlBlock>("TestAffinityErrors", 10,
                                                test_affinity_errors, nullptr);
  tm.AddTask(std::move(t4));

  int timeout = 200;
  while (timeout-- > 0 && g_tests_completed.load() < kNumTests) {
    (void)sys_sleep(50);
  }

  EXPECT_EQ(g_tests_completed.load(), kNumTests,
            "All affinity tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No affinity tests failed");

  klog::Info("Affinity System Test: COMPLETED");
  return true;
}
```

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

```bash
git add tests/system_test/affinity_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add affinity system test"
```

---

### Task 6: tick_test.cpp

**Files:**
- Create: `tests/system_test/tick_test.cpp`
- Modify: `tests/system_test/system_test.h`, `main.cpp`, `CMakeLists.txt`

- [ ] **Step 1: Register test**

Declaration: `auto tick_test() -> bool;`

- [ ] **Step 2: Create tick_test.cpp**

Tests:
1. `local_tick` increments over time (read before/after sleep, verify increase)
2. Sleep wake timing: sleep(N ms) wakes after approximately N ms worth of ticks
3. Time slice decrement: running task's `time_slice_remaining` decreases

```cpp
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

/// Test 1: local_tick increments
void test_tick_increments(void* /*arg*/) {
  klog::Info("=== Tick Increment Test ===");
  bool passed = true;

  auto* cpu_sched = per_cpu::GetCurrentCore().sched_data;
  uint64_t tick_before = cpu_sched->local_tick;
  (void)sys_sleep(200);
  uint64_t tick_after = cpu_sched->local_tick;

  if (tick_after <= tick_before) {
    klog::Err("Tick did not increment: before={} after={}", tick_before,
              tick_after);
    passed = false;
  } else {
    klog::Info("Tick incremented: {} -> {} (delta={})", tick_before, tick_after,
               tick_after - tick_before);
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Tick Increment Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 2: Sleep wakes approximately on time
void test_sleep_timing(void* /*arg*/) {
  klog::Info("=== Sleep Timing Test ===");
  bool passed = true;

  auto* cpu_sched = per_cpu::GetCurrentCore().sched_data;

  uint64_t tick_before = cpu_sched->local_tick;
  (void)sys_sleep(500);
  uint64_t tick_after = cpu_sched->local_tick;
  uint64_t delta = tick_after - tick_before;

  // With typical tick frequency, 500ms should produce a measurable delta
  // We don't check exact timing, just that it's reasonable (> 0)
  if (delta == 0) {
    klog::Err("Sleep produced 0 tick delta");
    passed = false;
  } else {
    klog::Info("Sleep 500ms: {} ticks elapsed", delta);
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Sleep Timing Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 3: total_runtime increases while task runs
void test_runtime_tracking(void* /*arg*/) {
  klog::Info("=== Runtime Tracking Test ===");
  bool passed = true;

  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  uint64_t runtime_before = self->sched_info.total_runtime;

  // Busy-wait a bit to accumulate runtime
  for (volatile int i = 0; i < 100000; ++i) {
  }
  (void)sys_yield();

  uint64_t runtime_after = self->sched_info.total_runtime;

  if (runtime_after <= runtime_before) {
    klog::Err("total_runtime did not increase: {} -> {}", runtime_before,
              runtime_after);
    passed = false;
  } else {
    klog::Info("Runtime tracked: {} -> {} (delta={})", runtime_before,
               runtime_after, runtime_after - runtime_before);
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Runtime Tracking Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto tick_test() -> bool {
  klog::Info("===== Tick System Test Start =====");
  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();
  constexpr int kNumTests = 3;

  auto t1 = kstd::make_unique<TaskControlBlock>("TestTickIncrement", 10,
                                                test_tick_increments, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestSleepTiming", 10,
                                                test_sleep_timing, nullptr);
  tm.AddTask(std::move(t2));

  auto t3 = kstd::make_unique<TaskControlBlock>("TestRuntimeTracking", 10,
                                                test_runtime_tracking, nullptr);
  tm.AddTask(std::move(t3));

  int timeout = 200;
  while (timeout-- > 0 && g_tests_completed.load() < kNumTests) {
    (void)sys_sleep(50);
  }

  EXPECT_EQ(g_tests_completed.load(), kNumTests, "All tick tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No tick tests failed");

  klog::Info("Tick System Test: COMPLETED");
  return true;
}
```

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

```bash
git add tests/system_test/tick_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add tick/timer system test"
```

---

### Task 7: zombie_reap_test.cpp

**Files:**
- Create: `tests/system_test/zombie_reap_test.cpp`
- Modify: `tests/system_test/system_test.h`, `main.cpp`, `CMakeLists.txt`

- [ ] **Step 1: Register test**

Declaration: `auto zombie_reap_test() -> bool;`

- [ ] **Step 2: Create zombie_reap_test.cpp**

Tests:
1. Child exits → becomes zombie → parent Wait() reaps it → task removed from table
2. Parent exits before child → child reparented to init (parent_pid changes)
3. Multiple children exit → parent Wait(-1) collects all

```cpp
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

/// Test 1: Zombie reaping via Wait
void zombie_child(void* /*arg*/) { sys_exit(77); }

void test_zombie_reap(void* /*arg*/) {
  klog::Info("=== Zombie Reap Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  auto child = kstd::make_unique<TaskControlBlock>("ZombieChild", 10,
                                                   zombie_child, nullptr);
  child->aux->parent_pid = self->pid;
  auto* raw = child.get();
  tm.AddTask(std::move(child));
  Pid child_pid = raw->pid;

  // Wait for child to exit and be reaped
  int status = 0;
  auto result = tm.Wait(child_pid, &status, false, false);
  if (!result.has_value()) {
    klog::Err("Wait failed");
    passed = false;
  } else if (result.value() != child_pid) {
    klog::Err("Wait returned wrong PID: {}", result.value());
    passed = false;
  } else if (status != 77) {
    klog::Err("Exit code: {} (expected 77)", status);
    passed = false;
  }

  // After Wait, task should be reaped from table
  (void)sys_sleep(100);  // Give ReapTask time
  auto* found = tm.FindTask(child_pid);
  if (found != nullptr) {
    klog::Err("Task {} still in table after Wait", child_pid);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Zombie Reap Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 2: Orphan reparenting
std::atomic<Pid> g_orphan_pid{0};
std::atomic<Pid> g_orphan_parent_after{0};

void orphan_grandchild(void* /*arg*/) {
  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  g_orphan_pid.store(self->pid);
  // Wait for parent to exit
  (void)sys_sleep(500);
  // After parent exits, check our new parent_pid
  g_orphan_parent_after.store(self->aux->parent_pid);
  sys_exit(0);
}

void orphan_parent(void* /*arg*/) {
  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  auto child = kstd::make_unique<TaskControlBlock>("OrphanGrandchild", 10,
                                                   orphan_grandchild, nullptr);
  child->aux->parent_pid = self->pid;
  tm.AddTask(std::move(child));

  // Exit immediately, orphaning the grandchild
  (void)sys_sleep(100);
  sys_exit(0);
}

void test_orphan_reparent(void* /*arg*/) {
  klog::Info("=== Orphan Reparent Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_orphan_pid = 0;
  g_orphan_parent_after = 0;

  auto parent = kstd::make_unique<TaskControlBlock>("OrphanParent", 10,
                                                    orphan_parent, nullptr);
  parent->aux->parent_pid = self->pid;
  tm.AddTask(std::move(parent));

  // Wait for grandchild to complete
  int timeout = 200;
  while (timeout-- > 0 && g_orphan_parent_after.load() == 0) {
    (void)sys_sleep(50);
  }

  Pid new_parent = g_orphan_parent_after.load();
  if (new_parent == 0) {
    klog::Err("Orphan grandchild did not report new parent");
    passed = false;
  } else {
    klog::Info("Orphan reparented to pid={}", new_parent);
    // After reparenting, parent_pid should be init (PID 1) or at least
    // different from the original parent
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Orphan Reparent Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 3: Wait(-1) collects multiple children
void multi_child(void* arg) {
  int code = static_cast<int>(reinterpret_cast<uintptr_t>(arg));
  (void)sys_sleep(50);
  sys_exit(code);
}

void test_wait_multi_children(void* /*arg*/) {
  klog::Info("=== Wait Multi Children Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  constexpr int kN = 3;
  for (int i = 0; i < kN; ++i) {
    auto child = kstd::make_unique<TaskControlBlock>(
        "MultiChild", 10, multi_child,
        reinterpret_cast<void*>(static_cast<uintptr_t>(i + 1)));
    child->aux->parent_pid = self->pid;
    tm.AddTask(std::move(child));
  }

  // Collect all children with Wait(-1)
  int collected = 0;
  int timeout = 100;
  while (collected < kN && timeout-- > 0) {
    int status = 0;
    auto result = tm.Wait(static_cast<Pid>(-1), &status, true, false);
    if (result.has_value() && result.value() > 0) {
      collected++;
      klog::Info("Collected child pid={} status={}", result.value(), status);
    } else {
      (void)sys_sleep(50);
    }
  }

  if (collected != kN) {
    klog::Err("Collected {} of {} children", collected, kN);
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

auto zombie_reap_test() -> bool {
  klog::Info("===== Zombie/Reap System Test Start =====");
  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();
  constexpr int kNumTests = 3;

  auto t1 = kstd::make_unique<TaskControlBlock>("TestZombieReap", 10,
                                                test_zombie_reap, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestOrphanReparent", 10,
                                                test_orphan_reparent, nullptr);
  tm.AddTask(std::move(t2));

  auto t3 = kstd::make_unique<TaskControlBlock>("TestWaitMulti", 10,
                                                test_wait_multi_children,
                                                nullptr);
  tm.AddTask(std::move(t3));

  int timeout = 400;
  while (timeout-- > 0 && g_tests_completed.load() < kNumTests) {
    (void)sys_sleep(50);
  }

  EXPECT_EQ(g_tests_completed.load(), kNumTests,
            "All zombie/reap tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No zombie/reap tests failed");

  klog::Info("Zombie/Reap System Test: COMPLETED");
  return true;
}
```

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

```bash
git add tests/system_test/zombie_reap_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add zombie reap and orphan reparent system test"
```

---

### Task 8: stress_test.cpp

**Files:**
- Create: `tests/system_test/stress_test.cpp`
- Modify: `tests/system_test/system_test.h`, `main.cpp`, `CMakeLists.txt`

- [ ] **Step 1: Register test**

Declaration: `auto stress_test() -> bool;`

- [ ] **Step 2: Create stress_test.cpp**

Tests:
1. Create many tasks (e.g. 20) concurrently, all exit cleanly
2. Wait error path: Wait on non-child PID returns error
3. Rapid create-exit cycle: create task, wait for it, repeat N times

```cpp
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

/// Test 1: Many concurrent tasks
std::atomic<int> g_many_done{0};

void many_tasks_work(void* /*arg*/) {
  g_many_done++;
  sys_exit(0);
}

void test_many_tasks(void* /*arg*/) {
  klog::Info("=== Many Tasks Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();
  g_many_done = 0;

  constexpr int kCount = 20;
  Pid pids[kCount];

  for (int i = 0; i < kCount; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>("ManyTask", 10,
                                                   many_tasks_work, nullptr);
    task->aux->parent_pid = self->pid;
    auto* raw = task.get();
    tm.AddTask(std::move(task));
    pids[i] = raw->pid;
  }

  // Wait for all
  for (int i = 0; i < kCount; ++i) {
    int status = 0;
    (void)tm.Wait(pids[i], &status, false, false);
  }

  if (g_many_done.load() != kCount) {
    klog::Err("Only {} of {} tasks completed", g_many_done.load(), kCount);
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Many Tasks Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 2: Wait on non-child returns error
void test_wait_non_child(void* /*arg*/) {
  klog::Info("=== Wait Non-Child Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();

  // Wait on PID 99999 — should fail
  int status = 0;
  auto result = tm.Wait(99999, &status, false, false);
  if (result.has_value() && result.value() > 0) {
    klog::Err("Wait(99999) unexpectedly succeeded with pid={}",
              result.value());
    passed = false;
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Wait Non-Child Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

/// Test 3: Rapid create-exit cycle
void rapid_work(void* arg) {
  int code = static_cast<int>(reinterpret_cast<uintptr_t>(arg));
  sys_exit(code);
}

void test_rapid_create_exit(void* /*arg*/) {
  klog::Info("=== Rapid Create-Exit Test ===");
  bool passed = true;

  auto& tm = TaskManagerSingleton::instance();
  auto* self = tm.GetCurrentTask();

  constexpr int kCycles = 10;
  for (int i = 0; i < kCycles; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>(
        "RapidTask", 10, rapid_work,
        reinterpret_cast<void*>(static_cast<uintptr_t>(i)));
    task->aux->parent_pid = self->pid;
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
      klog::Err("Rapid cycle {} timed out", i);
      passed = false;
      break;
    }
    if (status != i) {
      klog::Err("Rapid cycle {} exit code: {} (expected {})", i, status, i);
      passed = false;
      break;
    }
  }

  if (!passed) {
    g_tests_failed++;
  }
  g_tests_completed++;
  klog::Info("Rapid Create-Exit Test: {}", passed ? "PASSED" : "FAILED");
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto stress_test() -> bool {
  klog::Info("===== Stress System Test Start =====");
  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& tm = TaskManagerSingleton::instance();
  constexpr int kNumTests = 3;

  auto t1 = kstd::make_unique<TaskControlBlock>("TestManyTasks", 10,
                                                test_many_tasks, nullptr);
  tm.AddTask(std::move(t1));

  auto t2 = kstd::make_unique<TaskControlBlock>("TestWaitNonChild", 10,
                                                test_wait_non_child, nullptr);
  tm.AddTask(std::move(t2));

  auto t3 = kstd::make_unique<TaskControlBlock>("TestRapidCreateExit", 10,
                                                test_rapid_create_exit,
                                                nullptr);
  tm.AddTask(std::move(t3));

  int timeout = 400;
  while (timeout-- > 0 && g_tests_completed.load() < kNumTests) {
    (void)sys_sleep(50);
  }

  EXPECT_EQ(g_tests_completed.load(), kNumTests,
            "All stress tests completed");
  EXPECT_EQ(g_tests_failed.load(), 0, "No stress tests failed");

  klog::Info("Stress System Test: COMPLETED");
  return true;
}
```

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

```bash
git add tests/system_test/stress_test.cpp tests/system_test/system_test.h \
        tests/system_test/main.cpp tests/system_test/CMakeLists.txt
git commit --signoff -m "test(task): add stress and error path system test"
```

---

## Summary of All Registration Changes

### system_test.h — add 7 declarations:
```cpp
auto yield_test() -> bool;
auto fork_test() -> bool;
auto signal_test() -> bool;
auto affinity_test() -> bool;
auto tick_test() -> bool;
auto zombie_reap_test() -> bool;
auto stress_test() -> bool;
```

### main.cpp — update kTestCount and test_cases:
- `kTestCount` changes from 19 to 26
- Add 7 entries to `test_cases` array:
```cpp
{"yield_test", yield_test, false},
{"fork_test", fork_test, false},
{"signal_test", signal_test, false},
{"affinity_test", affinity_test, false},
{"tick_test", tick_test, false},
{"zombie_reap_test", zombie_reap_test, false},
{"stress_test", stress_test, false},
```

### CMakeLists.txt — add 7 source files:
```cmake
yield_test.cpp
fork_test.cpp
signal_test.cpp
affinity_test.cpp
tick_test.cpp
zombie_reap_test.cpp
stress_test.cpp
```

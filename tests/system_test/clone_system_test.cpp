/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kstd_cstdio"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "sk_stdlib.h"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

std::atomic<Pid> g_child1_pid{0};
std::atomic<Pid> g_child1_tgid{0};
std::atomic<Pid> g_child1_parent_pid{0};
std::atomic<Pid> g_child2_pid{0};
std::atomic<Pid> g_child2_tgid{0};
std::atomic<Pid> g_child2_parent_pid{0};
std::atomic<int> g_process_done{0};

std::atomic<Pid> g_leader_pid{0};
std::atomic<Pid> g_thread1_pid{0};
std::atomic<Pid> g_thread1_tgid{0};
std::atomic<Pid> g_thread2_pid{0};
std::atomic<Pid> g_thread2_tgid{0};
std::atomic<int> g_thread_done{0};
std::atomic<bool> g_threads_may_exit{false};

std::atomic<uint64_t> g_flags_value{0};
std::atomic<int> g_flags_done{0};

void child_process_work(void* arg) {
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  uint64_t child_id = reinterpret_cast<uint64_t>(arg);
  if (child_id == 1) {
    g_child1_pid.store(current->pid);
    g_child1_tgid.store(current->aux->tgid);
    g_child1_parent_pid.store(current->aux->parent_pid);
  } else {
    g_child2_pid.store(current->pid);
    g_child2_tgid.store(current->aux->tgid);
    g_child2_parent_pid.store(current->aux->parent_pid);
  }
  g_process_done++;
  sys_exit(0);
}

void child_thread_work(void* arg) {
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  uint64_t thread_id = reinterpret_cast<uint64_t>(arg);
  if (thread_id == 1) {
    g_thread1_pid.store(current->pid);
    g_thread1_tgid.store(current->aux->tgid);
  } else {
    g_thread2_pid.store(current->pid);
    g_thread2_tgid.store(current->aux->tgid);
  }
  g_thread_done++;
  while (!g_threads_may_exit.load()) {
    (void)sys_sleep(10);
  }
  sys_exit(0);
}

void leader_work(void* /*arg*/) {
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  g_leader_pid.store(current->pid);
  (void)sys_sleep(500);
  sys_exit(0);
}

void flags_reporter_work(void* /*arg*/) {
  auto* current = TaskManagerSingleton::instance().GetCurrentTask();
  g_flags_value.store(current->aux->clone_flags.value());
  g_flags_done++;
  sys_exit(0);
}

void noop_work(void* /*arg*/) { sys_exit(0); }

void test_clone_process(void* /*arg*/) {
  klog::Info("=== Clone Process Test ===");

  bool passed = true;
  g_process_done = 0;
  g_child1_pid = 0;
  g_child1_tgid = 0;
  g_child1_parent_pid = 0;
  g_child2_pid = 0;
  g_child2_tgid = 0;
  g_child2_parent_pid = 0;

  auto* self = TaskManagerSingleton::instance().GetCurrentTask();
  Pid my_pid = self->pid;

  auto child1 = kstd::make_unique<TaskControlBlock>(
      "CloneChild1", 10, child_process_work, reinterpret_cast<void*>(1));
  child1->aux->parent_pid = my_pid;
  TaskManagerSingleton::instance().AddTask(std::move(child1));

  auto child2 = kstd::make_unique<TaskControlBlock>(
      "CloneChild2", 10, child_process_work, reinterpret_cast<void*>(2));
  child2->aux->parent_pid = my_pid;
  TaskManagerSingleton::instance().AddTask(std::move(child2));

  (void)sys_sleep(200);

  Pid c1_pid = g_child1_pid.load();
  Pid c1_tgid = g_child1_tgid.load();
  Pid c2_pid = g_child2_pid.load();
  Pid c2_tgid = g_child2_tgid.load();

  if (c1_pid == 0 || c2_pid == 0) {
    klog::Err("Child processes did not start");
    passed = false;
  }

  if (passed && (c1_tgid != c1_pid || c2_tgid != c2_pid)) {
    klog::Err("Child tgid != pid: c1 tgid={} pid={}, c2 tgid={} pid={}",
              c1_tgid, c1_pid, c2_tgid, c2_pid);
    passed = false;
  }

  if (passed && (g_child1_parent_pid.load() != my_pid ||
                 g_child2_parent_pid.load() != my_pid)) {
    klog::Err("Parent-child relationship incorrect");
    passed = false;
  }

  if (passed && g_process_done.load() < 2) {
    klog::Err("Not all child processes completed");
    passed = false;
  }

  if (passed) {
    klog::Info("Clone Process Test: PASSED");
  } else {
    klog::Err("Clone Process Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

void test_clone_thread(void* /*arg*/) {
  klog::Info("=== Clone Thread Test ===");

  bool passed = true;
  g_thread_done = 0;
  g_threads_may_exit = false;
  g_leader_pid = 0;
  g_thread1_pid = 0;
  g_thread1_tgid = 0;
  g_thread2_pid = 0;
  g_thread2_tgid = 0;

  auto leader_ptr = kstd::make_unique<TaskControlBlock>("CloneThreadLeader", 10,
                                                        leader_work, nullptr);
  TaskManagerSingleton::instance().AddTask(std::move(leader_ptr));

  for (int i = 0; i < 100 && g_leader_pid.load() == 0; ++i) {
    (void)sys_sleep(10);
  }

  Pid leader_pid = g_leader_pid.load();
  if (leader_pid == 0) {
    klog::Err("Leader did not start");
    g_tests_failed++;
    g_tests_completed++;
    sys_exit(1);
  }

  auto* leader = TaskManagerSingleton::instance().FindTask(leader_pid);
  if (!leader) {
    klog::Err("Leader not found in task table");
    g_tests_failed++;
    g_tests_completed++;
    sys_exit(1);
  }

  uint64_t flags = clone_flag::kThread | clone_flag::kVm | clone_flag::kFiles |
                   clone_flag::kSighand;

  auto thread1 = kstd::make_unique<TaskControlBlock>(
      "CloneThread1", 10, child_thread_work, reinterpret_cast<void*>(1));
  thread1->aux->parent_pid = leader_pid;
  thread1->aux->tgid = leader_pid;
  thread1->aux->clone_flags = static_cast<CloneFlags>(flags);
  thread1->JoinThreadGroup(leader);
  TaskManagerSingleton::instance().AddTask(std::move(thread1));

  auto thread2 = kstd::make_unique<TaskControlBlock>(
      "CloneThread2", 10, child_thread_work, reinterpret_cast<void*>(2));
  thread2->aux->parent_pid = leader_pid;
  thread2->aux->tgid = leader_pid;
  thread2->aux->clone_flags = static_cast<CloneFlags>(flags);
  thread2->JoinThreadGroup(leader);
  TaskManagerSingleton::instance().AddTask(std::move(thread2));

  for (int i = 0; i < 100 && g_thread_done.load() < 2; ++i) {
    (void)sys_sleep(10);
  }

  Pid t1_tgid = g_thread1_tgid.load();
  Pid t2_tgid = g_thread2_tgid.load();

  if (t1_tgid != leader_pid || t2_tgid != leader_pid) {
    klog::Err("Thread tgid mismatch: t1={} t2={} expected={}", t1_tgid, t2_tgid,
              leader_pid);
    passed = false;
  }

  if (g_thread1_pid.load() == g_thread2_pid.load()) {
    klog::Err("Threads have identical PIDs");
    passed = false;
  }

  size_t group_size = leader->GetThreadGroupSize();
  if (group_size != 3) {
    klog::Err("Thread group size is {}, expected 3", group_size);
    passed = false;
  }

  g_threads_may_exit = true;

  if (passed) {
    klog::Info("Clone Thread Test: PASSED");
  } else {
    klog::Err("Clone Thread Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

void test_clone_parent_flag(void* /*arg*/) {
  klog::Info("=== Clone Parent Flag Test ===");

  bool passed = true;

  {
    auto task = kstd::make_unique<TaskControlBlock>("DefaultTask", 10,
                                                    noop_work, nullptr);
    if (task->aux->parent_pid != 0) {
      klog::Err("Default parent_pid is not 0");
      passed = false;
    }
    if (task->aux->tgid != 0) {
      klog::Err("Default tgid is not 0");
      passed = false;
    }
    if (task->aux->clone_flags.value() != 0) {
      klog::Err("Default clone_flags is not empty");
      passed = false;
    }

    auto* raw = task.get();
    TaskManagerSingleton::instance().AddTask(std::move(task));

    if (raw->pid == 0) {
      klog::Err("AddTask did not assign pid");
      passed = false;
    }
    if (raw->aux->tgid != raw->pid) {
      klog::Err("AddTask did not set tgid=pid: tgid={} pid={}", raw->aux->tgid,
                raw->pid);
      passed = false;
    }
  }

  {
    auto leader = kstd::make_unique<TaskControlBlock>("JoinLeader", 10,
                                                      noop_work, nullptr);
    auto* leader_raw = leader.get();
    TaskManagerSingleton::instance().AddTask(std::move(leader));

    auto member = kstd::make_unique<TaskControlBlock>("JoinMember", 10,
                                                      noop_work, nullptr);
    member->JoinThreadGroup(leader_raw);
    auto* member_raw = member.get();
    TaskManagerSingleton::instance().AddTask(std::move(member));

    if (member_raw->aux->tgid != leader_raw->aux->tgid) {
      klog::Err("JoinThreadGroup did not link tgid: member={} leader={}",
                member_raw->aux->tgid, leader_raw->aux->tgid);
      passed = false;
    }
  }

  {
    auto task = kstd::make_unique<TaskControlBlock>("FieldTest", 10, noop_work,
                                                    nullptr);
    task->aux->parent_pid = 42;
    task->aux->pgid = 99;
    task->aux->sid = 7;
    task->aux->clone_flags =
        static_cast<CloneFlags>(clone_flag::kThread | clone_flag::kVm);

    if (task->aux->parent_pid != 42) {
      klog::Err("clone_flags assignment corrupted parent_pid");
      passed = false;
    }
    if (task->aux->pgid != 99) {
      klog::Err("clone_flags assignment corrupted pgid");
      passed = false;
    }
    if (task->aux->sid != 7) {
      klog::Err("clone_flags assignment corrupted sid");
      passed = false;
    }
    if (!(task->aux->clone_flags & clone_flag::kThread)) {
      klog::Err("kThread flag not set");
      passed = false;
    }
    if (!(task->aux->clone_flags & clone_flag::kVm)) {
      klog::Err("kVm flag not set");
      passed = false;
    }

    TaskManagerSingleton::instance().AddTask(std::move(task));
  }

  if (passed) {
    klog::Info("Clone Parent Flag Test: PASSED");
  } else {
    klog::Err("Clone Parent Flag Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

void test_clone_flags_auto_completion(void* /*arg*/) {
  klog::Info("=== Clone Flags Auto Completion Test ===");

  bool passed = true;
  g_flags_done = 0;
  g_flags_value = 0;

  {
    CloneFlags empty_flags{};
    if (empty_flags.value() != 0) {
      klog::Err("Default CloneFlags is not 0");
      passed = false;
    }

    CloneFlags combined = static_cast<CloneFlags>(
        clone_flag::kThread | clone_flag::kVm | clone_flag::kFiles);
    if (!(combined & clone_flag::kThread)) {
      klog::Err("kThread not set in combined flags");
      passed = false;
    }
    if (!(combined & clone_flag::kVm)) {
      klog::Err("kVm not set in combined flags");
      passed = false;
    }
    if (!(combined & clone_flag::kFiles)) {
      klog::Err("kFiles not set in combined flags");
      passed = false;
    }
    if (combined & clone_flag::kSighand) {
      klog::Err("kSighand unexpectedly set");
      passed = false;
    }
    if (combined & clone_flag::kParent) {
      klog::Err("kParent unexpectedly set");
      passed = false;
    }
  }

  {
    uint64_t expected_flags =
        clone_flag::kThread | clone_flag::kVm | clone_flag::kSighand;
    auto task = kstd::make_unique<TaskControlBlock>(
        "FlagsTask", 10, flags_reporter_work, nullptr);
    task->aux->clone_flags = static_cast<CloneFlags>(expected_flags);
    TaskManagerSingleton::instance().AddTask(std::move(task));

    for (int i = 0; i < 100 && g_flags_done.load() == 0; ++i) {
      (void)sys_sleep(10);
    }

    uint64_t reported = g_flags_value.load();
    if (reported != expected_flags) {
      klog::Err(
          "Flags not preserved through AddTask: got 0x{:x} expected "
          "0x{:x}",
          reported, expected_flags);
      passed = false;
    }
  }

  if (passed) {
    klog::Info("Clone Flags Auto Completion Test: PASSED");
  } else {
    klog::Err("Clone Flags Auto Completion Test: FAILED");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

}  // namespace

auto clone_system_test() -> bool {
  klog::Info("===== Clone System Test Start =====");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& task_mgr = TaskManagerSingleton::instance();

  auto test1 = kstd::make_unique<TaskControlBlock>("TestCloneProcess", 10,
                                                   test_clone_process, nullptr);
  task_mgr.AddTask(std::move(test1));

  auto test2 = kstd::make_unique<TaskControlBlock>("TestCloneThread", 10,
                                                   test_clone_thread, nullptr);
  task_mgr.AddTask(std::move(test2));

  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestCloneParentFlag", 10, test_clone_parent_flag, nullptr);
  task_mgr.AddTask(std::move(test3));

  auto test4 = kstd::make_unique<TaskControlBlock>(
      "TestCloneFlagsAutoCompletion", 10, test_clone_flags_auto_completion,
      nullptr);
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

  klog::Info("Clone System Test Suite: COMPLETED");
  return true;
}

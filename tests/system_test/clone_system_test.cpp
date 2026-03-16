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

std::atomic<int> g_process_counter{0};
std::atomic<int> g_thread_counter{0};
std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};

/**
 * @brief 子进程工作函数
 */
void child_process_work(void* arg) {
  uint64_t child_id = reinterpret_cast<uint64_t>(arg);

  klog::Info("Child process {}: starting", child_id);

  for (int i = 0; i < 5; ++i) {
    g_process_counter++;
    klog::Debug("Child process {}: counter={}, iter={}", child_id,
                g_process_counter.load(), i);
    (void)sys_sleep(20);
  }

  klog::Info("Child process {}: exiting", child_id);
  sys_exit(static_cast<int>(child_id));
}

/**
 * @brief 子线程工作函数
 */
void child_thread_work(void* arg) {
  uint64_t thread_id = reinterpret_cast<uint64_t>(arg);

  klog::Info("Child thread {}: starting", thread_id);

  for (int i = 0; i < 5; ++i) {
    g_thread_counter++;
    klog::Debug("Child thread {}: counter={}, iter={}", thread_id,
                g_thread_counter.load(), i);
    (void)sys_sleep(20);
  }

  klog::Info("Child thread {}: exiting", thread_id);
  sys_exit(static_cast<int>(thread_id));
}

/**
 * @brief 测试使用 clone 创建子进程（不共享地址空间）
 */
void test_clone_process(void* /*arg*/) {
  klog::Info("=== Clone Process Test ===");

  bool passed = true;
  g_process_counter = 0;

  // 创建父进程
  auto parent_ptr =
      kstd::make_unique<TaskControlBlock>("CloneParent", 10, nullptr, nullptr);
  parent_ptr->pid = 2000;
  parent_ptr->aux->tgid = 2000;
  parent_ptr->aux->parent_pid = 1;
  Pid parent_pid = parent_ptr->pid;
  Pid parent_pgid = parent_ptr->aux->pgid;
  TaskManagerSingleton::instance().AddTask(std::move(parent_ptr));

  // 使用 clone 创建子进程（不设置 kCloneVm，表示不共享地址空间）
  uint64_t flags = 0;  // 不共享地址空间

  // 创建子进程 1
  auto child1 = kstd::make_unique<TaskControlBlock>(
      "CloneChild1", 10, child_process_work, reinterpret_cast<void*>(1));
  child1->aux->parent_pid = parent_pid;
  child1->aux->pgid = parent_pgid;
  child1->aux->clone_flags = static_cast<CloneFlags>(flags);
  Pid child1_pid = child1->pid;
  Pid child1_tgid = child1->aux->tgid;
  Pid child1_parent_pid = child1->aux->parent_pid;

  TaskManagerSingleton::instance().AddTask(std::move(child1));

  // 创建子进程 2
  auto child2 = kstd::make_unique<TaskControlBlock>(
      "CloneChild2", 10, child_process_work, reinterpret_cast<void*>(2));
  child2->aux->parent_pid = parent_pid;
  child2->aux->pgid = parent_pgid;
  child2->aux->clone_flags = static_cast<CloneFlags>(flags);
  Pid child2_pid = child2->pid;
  Pid child2_tgid = child2->aux->tgid;
  Pid child2_parent_pid = child2->aux->parent_pid;

  TaskManagerSingleton::instance().AddTask(std::move(child2));

  klog::Info("Created parent (pid={}) and 2 child processes", parent_pid);

  // 等待子进程运行
  (void)sys_sleep(200);

  klog::Info("Process counter: {} (expected >= 10)", g_process_counter.load());

  // 验证子进程的 tgid 应该等于它们自己的 pid（独立进程）
  if (child1_tgid == child1_pid && child2_tgid == child2_pid) {
    klog::Info("Child processes have correct tgid");
  } else {
    klog::Err("Child processes have incorrect tgid");
    passed = false;
    g_tests_failed++;
  }

  // 验证父子关系
  if (child1_parent_pid == parent_pid && child2_parent_pid == parent_pid) {
    klog::Info("Parent-child relationship is correct");
  } else {
    klog::Err("Parent-child relationship is incorrect");
    passed = false;
    g_tests_failed++;
  }

  if (passed) {
    klog::Info("Clone Process Test: PASSED");
  } else {
    klog::Err("Clone Process Test: FAILED");
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

/**
 * @brief 测试使用 clone 创建线程（共享地址空间）
 */
void test_clone_thread(void* /*arg*/) {
  klog::Info("=== Clone Thread Test ===");

  bool passed = true;
  g_thread_counter = 0;

  // 创建父线程（线程组的主线程）
  auto leader_ptr = kstd::make_unique<TaskControlBlock>("CloneThreadLeader", 10,
                                                        nullptr, nullptr);
  leader_ptr->pid = 3000;
  leader_ptr->aux->tgid = 3000;
  leader_ptr->aux->parent_pid = 1;
  Pid leader_pid = leader_ptr->pid;
  Pid leader_tgid = leader_ptr->aux->tgid;
  Pid leader_pgid = leader_ptr->aux->pgid;
  auto* leader = leader_ptr.get();
  TaskManagerSingleton::instance().AddTask(std::move(leader_ptr));

  // 使用 clone 创建线程（设置 kCloneThread 和 kCloneVm）
  uint64_t flags = clone_flag::kThread | clone_flag::kVm | clone_flag::kFiles |
                   clone_flag::kSighand;

  // 创建子线程 1
  auto thread1 = kstd::make_unique<TaskControlBlock>(
      "CloneThread1", 10, child_thread_work, reinterpret_cast<void*>(1));
  thread1->aux->parent_pid = leader_pid;
  thread1->aux->tgid = leader_tgid;  // 共享线程组 ID
  thread1->aux->pgid = leader_pgid;
  thread1->aux->clone_flags = static_cast<CloneFlags>(flags);
  thread1->JoinThreadGroup(leader);
  Pid thread1_tgid = thread1->aux->tgid;

  TaskManagerSingleton::instance().AddTask(std::move(thread1));

  // 创建子线程 2
  auto thread2 = kstd::make_unique<TaskControlBlock>(
      "CloneThread2", 10, child_thread_work, reinterpret_cast<void*>(2));
  thread2->aux->parent_pid = leader_pid;
  thread2->aux->tgid = leader_tgid;  // 共享线程组 ID
  thread2->aux->pgid = leader_pgid;
  thread2->aux->clone_flags = static_cast<CloneFlags>(flags);
  thread2->JoinThreadGroup(leader);
  Pid thread2_tgid = thread2->aux->tgid;

  TaskManagerSingleton::instance().AddTask(std::move(thread2));

  klog::Info("Created thread leader (pid={}, tgid={}) and 2 threads",
             leader_pid, leader_tgid);

  // 等待线程运行
  (void)sys_sleep(200);

  klog::Info("Thread counter: {} (expected >= 10)", g_thread_counter.load());

  // 验证所有线程的 tgid 应该相同（属于同一线程组）
  if (thread1_tgid == leader_tgid && thread2_tgid == leader_tgid) {
    klog::Info("All threads have same tgid");
  } else {
    klog::Err("Threads have incorrect tgid");
    passed = false;
    g_tests_failed++;
  }

  // 验证线程组大小
  size_t group_size = leader->GetThreadGroupSize();
  klog::Info("Thread group size: {} (expected 3)", group_size);
  if (group_size == 3) {
    klog::Info("Thread group size is correct");
  } else {
    klog::Err("Thread group size is incorrect");
    passed = false;
    g_tests_failed++;
  }

  if (passed) {
    klog::Info("Clone Thread Test: PASSED");
  } else {
    klog::Err("Clone Thread Test: FAILED");
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

/**
 * @brief 测试 kCloneParent 标志
 */
void test_clone_parent_flag(void* /*arg*/) {
  klog::Info("=== Clone Parent Flag Test ===");

  bool passed = true;

  // 创建祖父进程
  auto* grandparent = new TaskControlBlock("Grandparent", 10, nullptr, nullptr);
  grandparent->pid = 4000;
  grandparent->aux->tgid = 4000;
  grandparent->aux->parent_pid = 1;

  // 创建父进程
  auto* parent = new TaskControlBlock("Parent", 10, nullptr, nullptr);
  parent->pid = 4001;
  parent->aux->tgid = 4001;
  parent->aux->parent_pid = grandparent->pid;

  // 不使用 kCloneParent：子进程的父进程应该是 parent
  auto* child_no_flag =
      new TaskControlBlock("ChildNoFlag", 10, nullptr, nullptr);
  child_no_flag->pid = 4002;
  child_no_flag->aux->tgid = 4002;
  child_no_flag->aux->parent_pid = parent->pid;  // 设置为 parent

  // 使用 kCloneParent：子进程的父进程应该是 grandparent
  uint64_t flags = clone_flag::kParent;
  auto* child_with_flag =
      new TaskControlBlock("ChildWithFlag", 10, nullptr, nullptr);
  child_with_flag->pid = 4003;
  child_with_flag->aux->tgid = 4003;
  child_with_flag->aux->parent_pid =
      parent->aux->parent_pid;  // 设置为 grandparent
  child_with_flag->aux->clone_flags = static_cast<CloneFlags>(flags);

  // 验证父进程关系
  bool check1 = (child_no_flag->aux->parent_pid == parent->pid);
  bool check2 = (child_with_flag->aux->parent_pid == grandparent->pid);

  klog::Info("Child without kCloneParent: parent_pid={} (expected {})",
             child_no_flag->aux->parent_pid, parent->pid);
  klog::Info("Child with kCloneParent: parent_pid={} (expected {})",
             child_with_flag->aux->parent_pid, grandparent->pid);

  if (check1 && check2) {
    klog::Info("kCloneParent flag works correctly");
  } else {
    klog::Err("kCloneParent flag test failed");
    passed = false;
    g_tests_failed++;
  }

  delete grandparent;
  delete parent;
  delete child_no_flag;
  delete child_with_flag;

  if (passed) {
    klog::Info("Clone Parent Flag Test: PASSED");
  } else {
    klog::Err("Clone Parent Flag Test: FAILED");
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

/**
 * @brief 测试 clone 时的标志位自动补全
 */
void test_clone_flags_auto_completion(void* /*arg*/) {
  klog::Info("=== Clone Flags Auto Completion Test ===");

  bool passed = true;

  // 如果只设置 kCloneThread，应该自动补全其他必需的标志
  uint64_t flags = clone_flag::kThread;

  if ((flags & clone_flag::kThread) &&
      (!(flags & clone_flag::kVm) || !(flags & clone_flag::kFiles) ||
       !(flags & clone_flag::kSighand))) {
    klog::Info("Auto-completing flags for kCloneThread");
    flags |= (clone_flag::kVm | clone_flag::kFiles | clone_flag::kSighand);
  }

  bool check1 = (flags & clone_flag::kThread);
  bool check2 = (flags & clone_flag::kVm);
  bool check3 = (flags & clone_flag::kFiles);
  bool check4 = (flags & clone_flag::kSighand);

  klog::Info("Flags after auto-completion: 0x{:x}", flags);

  if (check1 && check2 && check3 && check4) {
    klog::Info("All required flags are set");
  } else {
    klog::Err("Flag auto-completion failed");
    passed = false;
    g_tests_failed++;
  }

  if (passed) {
    klog::Info("Clone Flags Auto Completion Test: PASSED");
  } else {
    klog::Err("Clone Flags Auto Completion Test: FAILED");
  }

  g_tests_completed++;
  sys_exit(passed ? 0 : 1);
}

}  // namespace

/**
 * @brief Clone 系统测试入口
 */
auto clone_system_test() -> bool {
  klog::Info("===== Clone System Test Start =====");

  g_tests_completed = 0;
  g_tests_failed = 0;

  auto& task_mgr = TaskManagerSingleton::instance();

  // 测试 1: Clone process
  auto test1 = kstd::make_unique<TaskControlBlock>("TestCloneProcess", 10,
                                                   test_clone_process, nullptr);
  task_mgr.AddTask(std::move(test1));

  // 测试 2: Clone thread
  auto test2 = kstd::make_unique<TaskControlBlock>("TestCloneThread", 10,
                                                   test_clone_thread, nullptr);
  task_mgr.AddTask(std::move(test2));

  // 测试 3: Clone parent flag
  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestCloneParentFlag", 10, test_clone_parent_flag, nullptr);
  task_mgr.AddTask(std::move(test3));

  // 测试 4: Clone flags auto completion
  auto test4 = kstd::make_unique<TaskControlBlock>(
      "TestCloneFlagsAutoCompletion", 10, test_clone_flags_auto_completion,
      nullptr);
  task_mgr.AddTask(std::move(test4));

  // 等待所有子测试完成（或超时）
  int timeout = 200;  // 200 * 50ms = 10s roughly
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

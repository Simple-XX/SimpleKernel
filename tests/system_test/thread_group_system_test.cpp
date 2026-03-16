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
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "kstd_memory"
#include "sk_stdlib.h"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

namespace {

std::atomic<int> g_thread_counter{0};
std::atomic<int> g_thread_completed{0};

std::atomic<int> g_tests_completed{0};
std::atomic<int> g_tests_failed{0};
/**
 * @brief 线程函数，增加计数器
 */
void thread_increment(void* arg) {
  uint64_t thread_id = reinterpret_cast<uint64_t>(arg);

  for (int i = 0; i < 10; ++i) {
    g_thread_counter++;
    klog::Debug("Thread {}: counter={}, iter={}", thread_id,
                g_thread_counter.load(), i);
    (void)sys_sleep(10);
  }

  g_thread_completed++;
  klog::Info("Thread {}: completed", thread_id);
  sys_exit(0);
}

/**
 * @brief 测试线程组的基本功能
 */
void test_thread_group_basic(void* /*arg*/) {
  klog::Info("=== Thread Group Basic Test ===");

  g_thread_counter = 0;
  g_thread_completed = 0;

  // 创建主线程
  auto* leader =
      new TaskControlBlock("ThreadGroupLeader", 10, nullptr, nullptr);
  leader->pid = 1000;
  leader->aux->tgid = 1000;

  // 创建并加入线程组的线程
  auto thread1 = kstd::make_unique<TaskControlBlock>(
      "Thread1", 10, thread_increment, reinterpret_cast<void*>(1));
  thread1->pid = 1001;
  thread1->JoinThreadGroup(leader);

  auto thread2 = kstd::make_unique<TaskControlBlock>(
      "Thread2", 10, thread_increment, reinterpret_cast<void*>(2));
  thread2->pid = 1002;
  thread2->JoinThreadGroup(leader);

  auto thread3 = kstd::make_unique<TaskControlBlock>(
      "Thread3", 10, thread_increment, reinterpret_cast<void*>(3));
  thread3->pid = 1003;
  thread3->JoinThreadGroup(leader);

  // 验证线程组大小
  size_t group_size = leader->GetThreadGroupSize();
  klog::Info("Thread group size: {} (expected 4)", group_size);

  // 验证所有线程在同一线程组
  if (leader->InSameThreadGroup(thread1.get()) &&
      leader->InSameThreadGroup(thread2.get()) &&
      leader->InSameThreadGroup(thread3.get())) {
    klog::Info("All threads are in the same thread group: PASS");
  } else {
    klog::Err("Thread group membership check failed: FAIL");
  }

  // 添加到调度器
  auto& task_mgr = TaskManagerSingleton::instance();
  task_mgr.AddTask(std::move(thread1));
  task_mgr.AddTask(std::move(thread2));
  task_mgr.AddTask(std::move(thread3));

  // 等待线程完成
  for (int i = 0; i < 200 && g_thread_completed < 3; ++i) {
    (void)sys_sleep(50);
  }

  klog::Info("Thread completed count: {} (expected 3)",
             g_thread_completed.load());
  klog::Info("Final counter value: {} (expected 30)", g_thread_counter.load());

  // 清理
  delete leader;

  bool passed = (g_thread_completed == 3 && g_thread_counter >= 30);
  if (passed) {
    klog::Info("Thread Group Basic Test: PASS");
  } else {
    klog::Err("Thread Group Basic Test: FAIL");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

/**
 * @brief 测试线程组的动态加入和离开
 */
void test_thread_group_dynamic(void* /*arg*/) {
  klog::Info("=== Thread Group Dynamic Test ===");

  // 创建主线程
  auto* leader = new TaskControlBlock("DynamicLeader", 10, nullptr, nullptr);
  leader->pid = 2000;
  leader->aux->tgid = 2000;

  // 创建线程池
  constexpr int kThreadCount = 5;
  TaskControlBlock* threads[kThreadCount];

  for (int i = 0; i < kThreadCount; ++i) {
    threads[i] = new TaskControlBlock("DynamicThread", 10, nullptr, nullptr);
    threads[i]->pid = 2001 + i;
  }

  // 动态加入
  klog::Info("Joining threads...");
  for (int i = 0; i < kThreadCount; ++i) {
    threads[i]->JoinThreadGroup(leader);
    size_t size = leader->GetThreadGroupSize();
    klog::Debug("After join {}: group size={}", i, size);
  }

  size_t final_size = leader->GetThreadGroupSize();
  klog::Info("Final group size: {} (expected {})", final_size,
             kThreadCount + 1);

  // 动态离开
  klog::Info("Leaving threads...");
  for (int i = 0; i < kThreadCount; ++i) {
    threads[i]->LeaveThreadGroup();
    size_t size = leader->GetThreadGroupSize();
    klog::Debug("After leave {}: group size={}", i, size);
  }

  size_t remaining_size = leader->GetThreadGroupSize();
  klog::Info("Remaining group size: {} (expected 1)", remaining_size);

  // 清理
  for (int i = 0; i < kThreadCount; ++i) {
    delete threads[i];
  }
  delete leader;

  bool passed = (final_size == static_cast<size_t>(kThreadCount + 1) &&
                 remaining_size == 1);
  if (passed) {
    klog::Info("Thread Group Dynamic Test: PASS");
  } else {
    klog::Err("Thread Group Dynamic Test: FAIL");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

/**
 * @brief 测试线程组成员同时退出
 */
void concurrent_exit_worker(void* arg) {
  uint64_t thread_id = reinterpret_cast<uint64_t>(arg);

  // 执行一些工作
  for (int i = 0; i < 5; ++i) {
    klog::Debug("ConcurrentExitWorker {}: iter={}", thread_id, i);
    (void)sys_sleep(20);
  }

  klog::Info("ConcurrentExitWorker {}: exiting", thread_id);
  g_thread_completed++;
  sys_exit(0);
}

void test_thread_group_concurrent_exit(void* /*arg*/) {
  klog::Info("=== Thread Group Concurrent Exit Test ===");

  g_thread_completed = 0;

  // 创建主线程
  auto* leader = new TaskControlBlock("ConcurrentLeader", 10, nullptr, nullptr);
  leader->pid = 3000;
  leader->aux->tgid = 3000;

  // 创建多个工作线程
  constexpr int kWorkerCount = 4;
  for (int i = 0; i < kWorkerCount; ++i) {
    auto worker = kstd::make_unique<TaskControlBlock>(
        "ConcurrentWorker", 10, concurrent_exit_worker,
        reinterpret_cast<void*>(i));
    worker->pid = 3001 + i;
    worker->JoinThreadGroup(leader);
    TaskManagerSingleton::instance().AddTask(std::move(worker));
  }

  klog::Info("Started {} worker threads", kWorkerCount);

  // 等待所有线程完成
  for (int i = 0; i < 100 && g_thread_completed < kWorkerCount; ++i) {
    (void)sys_sleep(50);
  }

  klog::Info("Completed threads: {} (expected {})", g_thread_completed.load(),
             kWorkerCount);

  delete leader;

  bool passed = (g_thread_completed == kWorkerCount);
  if (passed) {
    klog::Info("Thread Group Concurrent Exit Test: PASS");
  } else {
    klog::Err("Thread Group Concurrent Exit Test: FAIL");
    g_tests_failed++;
  }

  g_tests_completed++;
  sys_exit(0);
}

}  // namespace

/**
 * @brief 线程组系统测试入口
 */
auto thread_group_system_test() -> bool {
  klog::Info("=== Thread Group System Test Suite ===");

  g_tests_completed = 0;
  g_tests_failed = 0;

  // 测试 1: 基本线程组功能
  auto test1 = kstd::make_unique<TaskControlBlock>(
      "TestThreadGroupBasic", 10, test_thread_group_basic, nullptr);
  TaskManagerSingleton::instance().AddTask(std::move(test1));

  // 测试 2: 动态加入和离开
  auto test2 = kstd::make_unique<TaskControlBlock>(
      "TestThreadGroupDynamic", 10, test_thread_group_dynamic, nullptr);
  TaskManagerSingleton::instance().AddTask(std::move(test2));

  // 测试 3: 并发退出
  auto test3 = kstd::make_unique<TaskControlBlock>(
      "TestThreadGroupConcurrentExit", 10, test_thread_group_concurrent_exit,
      nullptr);
  TaskManagerSingleton::instance().AddTask(std::move(test3));

  // 同步等待所有测试完成
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
            "All thread group tests should complete");
  EXPECT_EQ(g_tests_failed.load(), 0, "No thread group tests should fail");

  klog::Info("Thread Group System Test Suite: COMPLETED");
  return true;
}

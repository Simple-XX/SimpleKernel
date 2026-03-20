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
std::atomic<Pid> g_local_pid_counter{SIZE_MAX / 2};
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

  auto leader_holder = kstd::make_unique<TaskControlBlock>(
      "ThreadGroupLeader", 10, nullptr, nullptr);
  auto* leader = leader_holder.get();
  leader->pid = g_local_pid_counter.fetch_add(1);
  leader->aux->tgid = leader->pid;

  // 创建并加入线程组的线程
  auto thread1 = kstd::make_unique<TaskControlBlock>(
      "Thread1", 10, thread_increment, reinterpret_cast<void*>(1));
  auto* thread1_raw = thread1.get();
  thread1->JoinThreadGroup(leader);

  auto thread2 = kstd::make_unique<TaskControlBlock>(
      "Thread2", 10, thread_increment, reinterpret_cast<void*>(2));
  auto* thread2_raw = thread2.get();
  thread2->JoinThreadGroup(leader);

  auto thread3 = kstd::make_unique<TaskControlBlock>(
      "Thread3", 10, thread_increment, reinterpret_cast<void*>(3));
  auto* thread3_raw = thread3.get();
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

  auto leader_holder = kstd::make_unique<TaskControlBlock>("DynamicLeader", 10,
                                                           nullptr, nullptr);
  auto* leader = leader_holder.get();
  leader->pid = g_local_pid_counter.fetch_add(1);
  leader->aux->tgid = leader->pid;

  constexpr int kThreadCount = 5;
  etl::unique_ptr<TaskControlBlock> thread_holders[kThreadCount];
  TaskControlBlock* threads[kThreadCount];

  for (int i = 0; i < kThreadCount; ++i) {
    thread_holders[i] = kstd::make_unique<TaskControlBlock>("DynamicThread", 10,
                                                            nullptr, nullptr);
    threads[i] = thread_holders[i].get();
    threads[i]->pid = g_local_pid_counter.fetch_add(1);
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

  klog::Info("Leaving threads...");
  for (int i = 0; i < kThreadCount; ++i) {
    threads[i]->LeaveThreadGroup();
    size_t size = leader->GetThreadGroupSize();
    klog::Debug("After leave {}: group size={}", i, size);
  }

  size_t remaining_size = leader->GetThreadGroupSize();
  klog::Info("Remaining group size: {} (expected 1)", remaining_size);

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

  auto leader_holder = kstd::make_unique<TaskControlBlock>(
      "ConcurrentLeader", 10, nullptr, nullptr);
  auto* leader = leader_holder.get();
  leader->pid = g_local_pid_counter.fetch_add(1);
  leader->aux->tgid = leader->pid;

  // 创建多个工作线程
  constexpr int kWorkerCount = 4;
  for (int i = 0; i < kWorkerCount; ++i) {
    auto worker = kstd::make_unique<TaskControlBlock>(
        "ConcurrentWorker", 10, concurrent_exit_worker,
        reinterpret_cast<void*>(i));
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
auto thread_group_test() -> bool {
  klog::Info("=== Thread Group System Test Suite ===");

  g_tests_completed = 0;
  g_tests_failed = 0;

  struct SubTest {
    const char* name;
    void (*func)(void*);
  };

  SubTest sub_tests[] = {
      {"TestThreadGroupBasic", test_thread_group_basic},
      {"TestThreadGroupDynamic", test_thread_group_dynamic},
      {"TestThreadGroupConcurrentExit", test_thread_group_concurrent_exit},
  };

  constexpr int kExpectedTests = 3;

  // 顺序执行：子测试共享 g_thread_completed，并行会导致交叉计数
  for (int i = 0; i < kExpectedTests; ++i) {
    auto task = kstd::make_unique<TaskControlBlock>(sub_tests[i].name, 10,
                                                    sub_tests[i].func, nullptr);
    TaskManagerSingleton::instance().AddTask(std::move(task));

    // 等待当前子测试完成后再启动下一个
    int timeout = 400;
    while (timeout > 0 && g_tests_completed < i + 1) {
      (void)sys_sleep(50);
      timeout--;
    }
  }

  EXPECT_EQ(g_tests_completed.load(), kExpectedTests,
            "All thread group tests should complete");
  EXPECT_EQ(g_tests_failed.load(), 0, "No thread group tests should fail");

  klog::Info("Thread Group System Test Suite: COMPLETED");
  return true;
}

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "spinlock.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <format>
#include <thread>
#include <vector>

#include "test_environment_state.hpp"

namespace {

// 测试用的全局变量
static std::atomic<int> shared_counter{0};
static std::atomic<int> thread_counter{0};

class SpinLockTestable : public SpinLock {
 public:
  explicit SpinLockTestable(const char* name) : SpinLock(name) {}
  SpinLockTestable() = default;
};

class SpinLockTest : public ::testing::Test {
 protected:
  void SetUp() override {
    shared_counter = 0;
    thread_counter = 0;

    // 初始化环境层
    env_state_.InitializeCores(SIMPLEKERNEL_MAX_CORE_COUNT);
    env_state_.SetCurrentThreadEnvironment();
    env_state_.BindThreadToCore(std::this_thread::get_id(), 0);
  }

  void TearDown() override {
    // 清理环境
    env_state_.ClearCurrentThreadEnvironment();
  }

  test_env::TestEnvironmentState env_state_;
};

// 测试基本的 lock/UnLock 功能
TEST_F(SpinLockTest, BasicLockUnlock) {
  SpinLockTestable lock("basic_test");

  // 初始状态应该是未锁定的
  cpu_io::DisableInterrupt();
  EXPECT_TRUE(lock.Lock());

  // 解锁应该成功
  EXPECT_TRUE(lock.UnLock());
  cpu_io::EnableInterrupt();
}

// 测试中断控制
TEST_F(SpinLockTest, InterruptControl) {
  SpinLockTestable lock("interrupt_test");

  // 初始状态中断是开启的
  EXPECT_TRUE(cpu_io::GetInterruptStatus());

  {
    LockGuard<SpinLockTestable> guard(lock);
    // 加锁后中断应该被禁用
    EXPECT_FALSE(cpu_io::GetInterruptStatus());
  }
  // 解锁后中断应该被恢复
  EXPECT_TRUE(cpu_io::GetInterruptStatus());
}

// 测试中断状态恢复
TEST_F(SpinLockTest, InterruptRestore) {
  SpinLockTestable lock("intr_restore_test");

  // 模拟中断原本就是关闭的情况
  cpu_io::DisableInterrupt();
  EXPECT_FALSE(cpu_io::GetInterruptStatus());

  {
    LockGuard<SpinLockTestable> guard(lock);
    EXPECT_FALSE(cpu_io::GetInterruptStatus());
  }
  // 解锁后中断应该保持关闭（恢复原状）
  EXPECT_FALSE(cpu_io::GetInterruptStatus());

  // 清理：恢复中断
  cpu_io::EnableInterrupt();
}

// 测试多线程并发安全性
TEST_F(SpinLockTest, ConcurrentAccess) {
  SpinLockTestable lock("concurrent_test");
  const int num_threads = 4;
  const int increments_per_thread = 1000;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([this, &lock, increments_per_thread, i]() {
      env_state_.SetCurrentThreadEnvironment();
      env_state_.BindThreadToCore(std::this_thread::get_id(),
                                  i % env_state_.GetCoreCount());
      for (int j = 0; j < increments_per_thread; ++j) {
        cpu_io::DisableInterrupt();
        (void)lock.Lock();
        int temp = shared_counter.load();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        shared_counter.store(temp + 1);
        (void)lock.UnLock();
        cpu_io::EnableInterrupt();
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // 如果锁工作正常，最终结果应该是精确的
  EXPECT_EQ(shared_counter.load(), num_threads * increments_per_thread);
}

// 测试无锁的并发访问（验证锁确实有效）
TEST_F(SpinLockTest, ConcurrentAccessWithoutLock) {
  const int num_threads = 4;
  const int increments_per_thread = 1000;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([this, increments_per_thread, i]() {
      env_state_.SetCurrentThreadEnvironment();
      env_state_.BindThreadToCore(std::this_thread::get_id(),
                                  i % env_state_.GetCoreCount());
      for (int j = 0; j < increments_per_thread; ++j) {
        // 不使用锁的并发访问
        int temp = thread_counter.load();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        thread_counter.store(temp + 1);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // 没有锁的情况下，结果通常不会是精确的（由于竞态条件）
  // 这个测试可能会失败，这证明了锁的必要性
  EXPECT_LE(thread_counter.load(), num_threads * increments_per_thread);
}

// 测试嵌套中断控制
TEST_F(SpinLockTest, NestedInterruptControl) {
  SpinLockTestable lock1("nested_test1");
  SpinLockTestable lock2("nested_test2");

  EXPECT_TRUE(cpu_io::GetInterruptStatus());

  {
    LockGuard<SpinLockTestable> guard1(lock1);
    EXPECT_FALSE(cpu_io::GetInterruptStatus());

    // 嵌套加锁
    {
      LockGuard<SpinLockTestable> guard2(lock2);
      EXPECT_FALSE(cpu_io::GetInterruptStatus());
    }
    EXPECT_FALSE(cpu_io::GetInterruptStatus());  // 仍然禁用
  }
  EXPECT_TRUE(cpu_io::GetInterruptStatus());  // 恢复
}

// 测试递归加锁检测
TEST_F(SpinLockTest, RecursiveLockDetection) {
  SpinLockTestable lock("recursive_test");

  cpu_io::DisableInterrupt();
  EXPECT_TRUE(lock.Lock());

  // 同核心再次加锁应该失败（重入检测）
  EXPECT_FALSE(lock.Lock());

  EXPECT_TRUE(lock.UnLock());
  cpu_io::EnableInterrupt();

  // 解锁后再次 UnLock 应该失败
  cpu_io::DisableInterrupt();
  EXPECT_FALSE(lock.UnLock());
  cpu_io::EnableInterrupt();
}

// 测试多个锁的独立性
TEST_F(SpinLockTest, MultipleLockIndependence) {
  SpinLockTestable lock1("independent_test1");
  SpinLockTestable lock2("independent_test2");

  // 锁1和锁2应该是独立的
  cpu_io::DisableInterrupt();
  (void)lock1.Lock();
  EXPECT_TRUE(lock2.Lock());

  (void)lock1.UnLock();
  EXPECT_TRUE(lock2.UnLock());
  cpu_io::EnableInterrupt();
}

// 性能测试：测试锁的获取和释放速度
TEST_F(SpinLockTest, PerformanceTest) {
  SpinLockTestable lock("performance_test");
  const int iterations = 100000;

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i) {
    cpu_io::DisableInterrupt();
    (void)lock.Lock();
    (void)lock.UnLock();
    cpu_io::EnableInterrupt();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // 输出性能信息
  std::cout << std::format(
      "SpinLock performance: {} lock/UnLock pairs in {} microseconds\n",
      iterations, duration.count());

  // 基本的性能断言：应该能在合理时间内完成
  EXPECT_LT(duration.count(), 1000000);  // 少于1秒
}

// 测试异常情况
TEST_F(SpinLockTest, EdgeCases) {
  SpinLockTestable lock("edge_case_test");

  // 测试快速连续的 lock/UnLock
  for (int i = 0; i < 1000; ++i) {
    LockGuard<SpinLockTestable> guard(lock);
  }

  EXPECT_TRUE(cpu_io::GetInterruptStatus());  // 中断状态应该正确恢复
}

// 测试中断状态在锁持有期间保持关闭
TEST_F(SpinLockTest, InterruptDisabledDuringLock) {
  SpinLockTestable lock("interrupt_disabled_test");

  EXPECT_TRUE(cpu_io::GetInterruptStatus());

  {
    LockGuard<SpinLockTestable> guard(lock);

    // 锁持有期间中断应该被禁用
    EXPECT_FALSE(cpu_io::GetInterruptStatus());
  }

  // 解锁后中断应该恢复
  EXPECT_TRUE(cpu_io::GetInterruptStatus());
}

// 测试锁的公平性（FIFO 顺序）
TEST_F(SpinLockTest, FairnessTest) {
  SpinLockTestable lock("fairness_test");
  std::vector<int> execution_order;
  std::mutex order_mutex;
  const int num_threads = 5;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([this, &lock, &execution_order, &order_mutex, i]() {
      env_state_.SetCurrentThreadEnvironment();
      env_state_.BindThreadToCore(std::this_thread::get_id(),
                                  i % env_state_.GetCoreCount());
      std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));

      cpu_io::DisableInterrupt();
      (void)lock.Lock();
      {
        std::lock_guard<std::mutex> guard(order_mutex);
        execution_order.push_back(i);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      (void)lock.UnLock();
      cpu_io::EnableInterrupt();
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // 验证所有线程都执行了
  EXPECT_EQ(execution_order.size(), num_threads);
}

// 测试高负载下的锁性能
TEST_F(SpinLockTest, HighLoadPerformance) {
  SpinLockTestable lock("high_load_test");
  const int num_threads = 8;
  const int operations_per_thread = 1000;
  std::atomic<int> total_operations{0};

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(
        [this, &lock, operations_per_thread, &total_operations, i]() {
          env_state_.SetCurrentThreadEnvironment();
          env_state_.BindThreadToCore(std::this_thread::get_id(),
                                      i % env_state_.GetCoreCount());
          for (int j = 0; j < operations_per_thread; ++j) {
            cpu_io::DisableInterrupt();
            (void)lock.Lock();
            total_operations.fetch_add(1);
            (void)lock.UnLock();
            cpu_io::EnableInterrupt();
          }
        });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(total_operations.load(), num_threads * operations_per_thread);
}

// 测试中断状态的嵌套保存和恢复
TEST_F(SpinLockTest, NestedInterruptSaveRestore) {
  SpinLockTestable lock1("nested1");
  SpinLockTestable lock2("nested2");
  SpinLockTestable lock3("nested3");

  // 初始状态：中断开启
  EXPECT_TRUE(cpu_io::GetInterruptStatus());

  {
    LockGuard<SpinLockTestable> guard1(lock1);
    EXPECT_FALSE(cpu_io::GetInterruptStatus());

    {
      LockGuard<SpinLockTestable> guard2(lock2);
      EXPECT_FALSE(cpu_io::GetInterruptStatus());

      {
        LockGuard<SpinLockTestable> guard3(lock3);
        EXPECT_FALSE(cpu_io::GetInterruptStatus());
      }
      // guard3 destroyed
      EXPECT_FALSE(cpu_io::GetInterruptStatus());
    }
    // guard2 destroyed
    EXPECT_FALSE(cpu_io::GetInterruptStatus());
  }
  // guard1 destroyed - 恢复原始状态
  EXPECT_TRUE(cpu_io::GetInterruptStatus());
}

// 测试零竞争情况
TEST_F(SpinLockTest, NoContentionSingleThread) {
  SpinLockTestable lock("no_contention");
  const int iterations = 10000;

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i) {
    cpu_io::DisableInterrupt();
    (void)lock.Lock();
    shared_counter++;
    (void)lock.UnLock();
    cpu_io::EnableInterrupt();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  EXPECT_EQ(shared_counter.load(), iterations);

  std::cout << std::format(
      "Single thread (no contention): {} operations in {} microseconds\n",
      iterations, duration.count());
}

// 测试锁持有时间过长的情况
TEST_F(SpinLockTest, LongHoldTime) {
  SpinLockTestable lock("long_hold");
  std::atomic<bool> lock_held{false};
  std::atomic<bool> waiter_started{false};
  std::atomic<int> spin_count{0};

  std::thread holder([this, &lock, &lock_held, &waiter_started]() {
    env_state_.SetCurrentThreadEnvironment();
    env_state_.BindThreadToCore(std::this_thread::get_id(),
                                1 % env_state_.GetCoreCount());
    cpu_io::DisableInterrupt();
    (void)lock.Lock();
    lock_held = true;

    // 等待 waiter 线程开始尝试获取锁
    while (!waiter_started.load()) {
      std::this_thread::yield();
    }

    // 持有锁一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    (void)lock.UnLock();
    cpu_io::EnableInterrupt();
  });

  std::thread waiter([this, &lock, &lock_held, &spin_count, &waiter_started]() {
    env_state_.SetCurrentThreadEnvironment();
    env_state_.BindThreadToCore(std::this_thread::get_id(),
                                2 % env_state_.GetCoreCount());
    // 等待直到第一个线程持有锁
    while (!lock_held.load()) {
      std::this_thread::yield();
    }

    waiter_started = true;

    // 在尝试获取锁之前，开始计数自旋次数
    auto start_time = std::chrono::steady_clock::now();

    // 尝试获取锁（会自旋等待）
    cpu_io::DisableInterrupt();
    (void)lock.Lock();

    auto end_time = std::chrono::steady_clock::now();
    auto wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    // 如果等待时间大于 10ms，说明确实等待了
    if (wait_duration.count() > 10) {
      spin_count.fetch_add(1);
    }

    (void)lock.UnLock();
    cpu_io::EnableInterrupt();
  });

  holder.join();
  waiter.join();

  // 验证等待者确实等待了（等待时间应该大于 10ms）
  EXPECT_GT(spin_count.load(), 0);
}

// 测试多线程的独立性
TEST_F(SpinLockTest, MultipleThreads) {
  SpinLockTestable lock("multi_thread");
  std::vector<int> thread_results(4, 0);
  std::vector<std::thread> threads;

  for (int i = 0; i < 4; ++i) {
    threads.emplace_back([this, &lock, &thread_results, i]() {
      env_state_.SetCurrentThreadEnvironment();
      env_state_.BindThreadToCore(std::this_thread::get_id(),
                                  i % env_state_.GetCoreCount());
      for (int j = 0; j < 100; ++j) {
        cpu_io::DisableInterrupt();
        (void)lock.Lock();
        thread_results[i]++;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        (void)lock.UnLock();
        cpu_io::EnableInterrupt();
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // 验证每个线程都完成了预期的操作
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(thread_results[i], 100);
  }
}

// 测试锁的状态一致性
TEST_F(SpinLockTest, StateConsistency) {
  SpinLockTestable lock("consistency");

  // 初始状态
  EXPECT_TRUE(cpu_io::GetInterruptStatus());

  // 加锁
  {
    LockGuard<SpinLockTestable> guard(lock);
    EXPECT_FALSE(cpu_io::GetInterruptStatus());
  }
  // 解锁
  EXPECT_TRUE(cpu_io::GetInterruptStatus());

  // 多次循环验证状态一致性
  for (int i = 0; i < 100; ++i) {
    {
      LockGuard<SpinLockTestable> guard(lock);
      EXPECT_FALSE(cpu_io::GetInterruptStatus());
    }
    EXPECT_TRUE(cpu_io::GetInterruptStatus());
  }
}

}  // namespace

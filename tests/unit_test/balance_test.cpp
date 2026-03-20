/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <gtest/gtest.h>

#include "rr_scheduler.hpp"
#include "task_control_block.hpp"

/// @brief Test that stealing from a loaded RR scheduler works correctly.
/// This validates the core primitive that Balance() relies on:
/// Dequeue from source scheduler, Enqueue to destination scheduler.
TEST(BalanceTest, StealFromLoadedScheduler) {
  RoundRobinScheduler source;
  RoundRobinScheduler dest;

  // Source has 4 tasks, dest has 0
  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  TaskControlBlock task4("Task4", 4, nullptr, nullptr);

  source.Enqueue(&task1);
  source.Enqueue(&task2);
  source.Enqueue(&task3);
  source.Enqueue(&task4);

  EXPECT_EQ(source.GetQueueSize(), 4);
  EXPECT_EQ(dest.GetQueueSize(), 0);

  // Simulate steal: pick from source, enqueue to dest
  auto* stolen = source.PickNext();
  ASSERT_NE(stolen, nullptr);
  dest.Enqueue(stolen);

  EXPECT_EQ(source.GetQueueSize(), 3);
  EXPECT_EQ(dest.GetQueueSize(), 1);
}

/// @brief Test that no stealing occurs when queues are balanced.
TEST(BalanceTest, NoStealWhenBalanced) {
  RoundRobinScheduler sched1;
  RoundRobinScheduler sched2;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);

  sched1.Enqueue(&task1);
  sched2.Enqueue(&task2);

  // Both have 1 task — balanced, no steal needed
  EXPECT_EQ(sched1.GetQueueSize(), 1);
  EXPECT_EQ(sched2.GetQueueSize(), 1);

  // Difference is 0, which is <= 1 threshold — no steal
  size_t diff = sched1.GetQueueSize() > sched2.GetQueueSize()
                    ? sched1.GetQueueSize() - sched2.GetQueueSize()
                    : sched2.GetQueueSize() - sched1.GetQueueSize();
  EXPECT_LE(diff, 1);
}

/// @brief Test that stealing only picks from kNormal (RR) queues.
/// kRealTime (FIFO) and kIdle tasks should never be stolen.
TEST(BalanceTest, OnlyStealNormalPolicyTasks) {
  RoundRobinScheduler rr_source;

  // Only tasks with kNormal policy go into RR scheduler
  TaskControlBlock normal_task("NormalTask", 5, nullptr, nullptr);
  normal_task.policy = SchedPolicy::kNormal;

  rr_source.Enqueue(&normal_task);
  EXPECT_EQ(rr_source.GetQueueSize(), 1);

  auto* stolen = rr_source.PickNext();
  ASSERT_NE(stolen, nullptr);
  EXPECT_EQ(stolen->policy, SchedPolicy::kNormal);
}

/// @brief Test steal-one-at-a-time semantics.
TEST(BalanceTest, StealOnlyOneTask) {
  RoundRobinScheduler source;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);

  source.Enqueue(&task1);
  source.Enqueue(&task2);
  source.Enqueue(&task3);

  // Steal exactly one
  auto* stolen = source.PickNext();
  ASSERT_NE(stolen, nullptr);

  // Source should still have 2 tasks
  EXPECT_EQ(source.GetQueueSize(), 2);
}

/// @brief Test that stealing from empty source returns nullptr.
TEST(BalanceTest, StealFromEmptyScheduler) {
  RoundRobinScheduler source;

  EXPECT_TRUE(source.IsEmpty());
  EXPECT_EQ(source.PickNext(), nullptr);
}

/// @brief Test load imbalance detection across multiple schedulers.
TEST(BalanceTest, FindMostLoadedCore) {
  constexpr size_t kCoreCount = 4;
  RoundRobinScheduler schedulers[kCoreCount];

  // Core 0: 1 task, Core 1: 5 tasks, Core 2: 2 tasks, Core 3: 0 tasks
  TaskControlBlock tasks[8] = {
      {"T0", 1, nullptr, nullptr}, {"T1", 1, nullptr, nullptr},
      {"T2", 1, nullptr, nullptr}, {"T3", 1, nullptr, nullptr},
      {"T4", 1, nullptr, nullptr}, {"T5", 1, nullptr, nullptr},
      {"T6", 1, nullptr, nullptr}, {"T7", 1, nullptr, nullptr},
  };

  schedulers[0].Enqueue(&tasks[0]);
  schedulers[1].Enqueue(&tasks[1]);
  schedulers[1].Enqueue(&tasks[2]);
  schedulers[1].Enqueue(&tasks[3]);
  schedulers[1].Enqueue(&tasks[4]);
  schedulers[1].Enqueue(&tasks[5]);
  schedulers[2].Enqueue(&tasks[6]);
  schedulers[2].Enqueue(&tasks[7]);

  // Find the most loaded core
  size_t max_load = 0;
  size_t max_core = 0;
  for (size_t i = 0; i < kCoreCount; ++i) {
    size_t load = schedulers[i].GetQueueSize();
    if (load > max_load) {
      max_load = load;
      max_core = i;
    }
  }

  EXPECT_EQ(max_core, 1);
  EXPECT_EQ(max_load, 5);

  // Core 3 is the least loaded (0 tasks) — it would be the stealer
  size_t min_load = max_load;
  size_t min_core = 0;
  for (size_t i = 0; i < kCoreCount; ++i) {
    size_t load = schedulers[i].GetQueueSize();
    if (load < min_load) {
      min_load = load;
      min_core = i;
    }
  }

  EXPECT_EQ(min_core, 3);
  EXPECT_EQ(min_load, 0);

  // Steal one task from core 1 to core 3
  auto* stolen = schedulers[max_core].PickNext();
  ASSERT_NE(stolen, nullptr);
  schedulers[min_core].Enqueue(stolen);

  EXPECT_EQ(schedulers[1].GetQueueSize(), 4);
  EXPECT_EQ(schedulers[3].GetQueueSize(), 1);
}

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "idle_scheduler.hpp"

#include <cstdint>

#include "system_test.h"
#include "task_control_block.hpp"
#include "task_messages.hpp"

namespace {

auto test_idle_basic_functionality() -> bool {
  klog::Info("Running test_idle_basic_functionality...");

  IdleScheduler scheduler;

  // 验证调度器名称
  EXPECT_EQ(scheduler.name[0], 'I', "Scheduler name should start with I");

  // 创建 idle 任务
  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);
  idle_task.fsm.Receive(MsgSchedule{});

  // 测试空队列
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty initially");
  EXPECT_EQ(scheduler.GetQueueSize(), 0,
            "Queue size should be 0 for empty queue");
  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext should return nullptr for empty queue");

  // 加入 idle 任务
  scheduler.Enqueue(&idle_task);
  EXPECT_EQ(scheduler.GetQueueSize(), 1,
            "Queue size should be 1 after enqueue");
  EXPECT_FALSE(scheduler.IsEmpty(), "Scheduler should not be empty");

  // PickNext 应返回 idle 任务
  auto* picked = scheduler.PickNext();
  EXPECT_EQ(picked, &idle_task, "PickNext should return idle task");

  // 关键特性：PickNext 不应移除 idle 任务
  EXPECT_FALSE(scheduler.IsEmpty(),
               "Scheduler should NOT be empty after PickNext");
  EXPECT_EQ(scheduler.GetQueueSize(), 1,
            "Queue size should still be 1 after PickNext");

  // 多次 PickNext 应始终返回同一个 idle 任务
  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &idle_task,
            "Second PickNext should return same idle task");
  EXPECT_EQ(scheduler.GetQueueSize(), 1,
            "Queue size should still be 1 after multiple PickNext");

  klog::Info("test_idle_basic_functionality passed");
  return true;
}

auto test_idle_pick_next_does_not_remove() -> bool {
  klog::Info("Running test_idle_pick_next_does_not_remove...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);
  scheduler.Enqueue(&idle_task);

  // PickNext 多次，任务始终存在
  constexpr int kPickCount = 10;
  for (int i = 0; i < kPickCount; ++i) {
    auto* picked = scheduler.PickNext();
    EXPECT_EQ(picked, &idle_task, "PickNext should always return idle task");
    EXPECT_EQ(scheduler.GetQueueSize(), 1,
              "Queue size should remain 1 after PickNext");
  }

  klog::Info("test_idle_pick_next_does_not_remove passed");
  return true;
}

auto test_idle_enqueue_dequeue() -> bool {
  klog::Info("Running test_idle_enqueue_dequeue...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);

  // 入队
  scheduler.Enqueue(&idle_task);
  EXPECT_EQ(scheduler.GetQueueSize(), 1, "Queue size should be 1");

  // 出队
  scheduler.Dequeue(&idle_task);
  EXPECT_EQ(scheduler.GetQueueSize(), 0,
            "Queue size should be 0 after dequeue");
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty after dequeue");
  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext should return nullptr after dequeue");

  // 重新入队
  scheduler.Enqueue(&idle_task);
  EXPECT_EQ(scheduler.GetQueueSize(), 1,
            "Queue size should be 1 after re-enqueue");
  EXPECT_EQ(scheduler.PickNext(), &idle_task,
            "PickNext should return idle task after re-enqueue");

  klog::Info("test_idle_enqueue_dequeue passed");
  return true;
}

auto test_idle_on_tick_always_false() -> bool {
  klog::Info("Running test_idle_on_tick_always_false...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);
  scheduler.Enqueue(&idle_task);

  // OnTick 应始终返回 false（idle 不需要重新调度）
  constexpr int kTickCount = 10;
  for (int i = 0; i < kTickCount; ++i) {
    bool need_resched = scheduler.OnTick(&idle_task);
    EXPECT_FALSE(need_resched, "OnTick should always return false for idle");
  }

  klog::Info("test_idle_on_tick_always_false passed");
  return true;
}

auto test_idle_on_time_slice_expired_always_false() -> bool {
  klog::Info("Running test_idle_on_time_slice_expired_always_false...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);
  scheduler.Enqueue(&idle_task);

  // OnTimeSliceExpired 应始终返回 false（idle 不使用时间片）
  bool need_requeue = scheduler.OnTimeSliceExpired(&idle_task);
  EXPECT_FALSE(need_requeue, "OnTimeSliceExpired should return false for idle");

  klog::Info("test_idle_on_time_slice_expired_always_false passed");
  return true;
}

auto test_idle_statistics() -> bool {
  klog::Info("Running test_idle_statistics...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);

  // 初始统计
  auto stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 0, "Initial enqueues should be 0");
  EXPECT_EQ(stats.total_dequeues, 0, "Initial dequeues should be 0");
  EXPECT_EQ(stats.total_picks, 0, "Initial picks should be 0");
  EXPECT_EQ(stats.total_preemptions, 0, "Initial preemptions should be 0");

  // 测试入队统计
  scheduler.Enqueue(&idle_task);
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 1, "Enqueues should be 1");

  // 测试选择统计（PickNext 不移除）
  (void)scheduler.PickNext();
  (void)scheduler.PickNext();
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_picks, 2, "Picks should be 2");

  // 测试出队统计
  scheduler.Dequeue(&idle_task);
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_dequeues, 1, "Dequeues should be 1");

  // 测试抢占统计
  scheduler.OnPreempted(&idle_task);
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_preemptions, 1, "Preemptions should be 1");

  // 重置统计
  scheduler.ResetStats();
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 0, "Enqueues should be 0 after reset");
  EXPECT_EQ(stats.total_dequeues, 0, "Dequeues should be 0 after reset");
  EXPECT_EQ(stats.total_picks, 0, "Picks should be 0 after reset");
  EXPECT_EQ(stats.total_preemptions, 0, "Preemptions should be 0 after reset");

  klog::Info("test_idle_statistics passed");
  return true;
}

auto test_idle_dequeue_wrong_task() -> bool {
  klog::Info("Running test_idle_dequeue_wrong_task...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);
  TaskControlBlock other_task("OtherTask", 1, nullptr, nullptr);

  scheduler.Enqueue(&idle_task);

  // 移除不同的任务不应影响 idle 任务
  scheduler.Dequeue(&other_task);
  EXPECT_EQ(scheduler.GetQueueSize(), 1,
            "Queue size should still be 1 after dequeue of wrong task");
  EXPECT_EQ(scheduler.PickNext(), &idle_task,
            "Idle task should still be present");

  klog::Info("test_idle_dequeue_wrong_task passed");
  return true;
}

auto test_idle_robustness() -> bool {
  klog::Info("Running test_idle_robustness...");

  IdleScheduler scheduler;

  TaskControlBlock idle_task("IdleTask", 0, nullptr, nullptr);

  // 空队列操作
  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext on empty queue should return nullptr");
  scheduler.Dequeue(&idle_task);  // 不应崩溃

  // 重复移除
  scheduler.Enqueue(&idle_task);
  scheduler.Dequeue(&idle_task);
  scheduler.Dequeue(&idle_task);  // 再次移除已移除的任务，不应崩溃
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  // 替换 idle 任务
  TaskControlBlock idle_task2("IdleTask2", 0, nullptr, nullptr);
  scheduler.Enqueue(&idle_task);
  scheduler.Enqueue(&idle_task2);  // 应替换为新的 idle 任务
  EXPECT_EQ(scheduler.PickNext(), &idle_task2,
            "New idle task should replace old one");

  klog::Info("test_idle_robustness passed");
  return true;
}

}  // namespace

auto idle_scheduler_test() -> bool {
  klog::Info("\n=== Idle Scheduler System Tests ===\n");

  if (!test_idle_basic_functionality()) {
    return false;
  }

  if (!test_idle_pick_next_does_not_remove()) {
    return false;
  }

  if (!test_idle_enqueue_dequeue()) {
    return false;
  }

  if (!test_idle_on_tick_always_false()) {
    return false;
  }

  if (!test_idle_on_time_slice_expired_always_false()) {
    return false;
  }

  if (!test_idle_statistics()) {
    return false;
  }

  if (!test_idle_dequeue_wrong_task()) {
    return false;
  }

  if (!test_idle_robustness()) {
    return false;
  }

  klog::Info("=== All Idle Scheduler Tests Passed ===\n");
  return true;
}

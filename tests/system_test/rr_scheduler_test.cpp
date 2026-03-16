/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "rr_scheduler.hpp"

#include <cstdint>

#include "system_test.h"
#include "task_control_block.hpp"
#include "task_messages.hpp"

namespace {

auto test_rr_basic_functionality() -> bool {
  klog::Info("Running test_rr_basic_functionality...");

  RoundRobinScheduler scheduler;

  // 创建测试任务
  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});

  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});

  // 测试空队列
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty initially");
  EXPECT_EQ(scheduler.GetQueueSize(), 0,
            "Queue size should be 0 for empty queue");
  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext should return nullptr for empty queue");

  // 加入任务
  scheduler.Enqueue(&task1);
  EXPECT_EQ(scheduler.GetQueueSize(), 1,
            "Queue size should be 1 after enqueue");

  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);
  EXPECT_EQ(scheduler.GetQueueSize(), 3,
            "Queue size should be 3 after 3 enqueues");

  // 验证 FIFO 顺序（第一轮）
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task1, "First picked task should be task1");

  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task2, "Second picked task should be task2");

  auto* picked3 = scheduler.PickNext();
  EXPECT_EQ(picked3, &task3, "Third picked task should be task3");

  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext should return nullptr after all tasks picked");

  klog::Info("test_rr_basic_functionality passed");
  return true;
}

auto test_rr_round_robin_behavior() -> bool {
  klog::Info("Running test_rr_round_robin_behavior...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});

  // 第一轮
  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);

  EXPECT_EQ(scheduler.PickNext(), &task1, "First round: task1");
  EXPECT_EQ(scheduler.PickNext(), &task2, "First round: task2");
  EXPECT_EQ(scheduler.PickNext(), &task3, "First round: task3");

  // 模拟时间片用完，任务重新入队（第二轮）
  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);

  EXPECT_EQ(scheduler.PickNext(), &task1, "Second round: task1");
  EXPECT_EQ(scheduler.PickNext(), &task2, "Second round: task2");
  EXPECT_EQ(scheduler.PickNext(), &task3, "Second round: task3");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty after 2 rounds");

  klog::Info("test_rr_round_robin_behavior passed");
  return true;
}

auto test_rr_time_slice_management() -> bool {
  klog::Info("Running test_rr_time_slice_management...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  task1.sched_info.time_slice_default = 20;
  task1.sched_info.time_slice_remaining = 5;  // 时间片快用完了

  // 入队应该重置时间片
  scheduler.Enqueue(&task1);
  EXPECT_EQ(task1.sched_info.time_slice_remaining, 20,
            "Enqueue should reset time slice");

  // 取出任务
  auto* picked = scheduler.PickNext();
  EXPECT_EQ(picked, &task1, "Should pick task1");

  // 模拟时间片耗尽
  task1.sched_info.time_slice_remaining = 0;
  bool should_reenqueue = scheduler.OnTimeSliceExpired(&task1);
  EXPECT_TRUE(should_reenqueue, "OnTimeSliceExpired should return true for RR");
  EXPECT_EQ(task1.sched_info.time_slice_remaining, 20,
            "OnTimeSliceExpired should reset time slice");

  klog::Info("test_rr_time_slice_management passed");
  return true;
}

auto test_rr_dequeue() -> bool {
  klog::Info("Running test_rr_dequeue...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});
  TaskControlBlock task4("Task4", 4, nullptr, nullptr);
  task4.fsm.Receive(MsgSchedule{});

  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);
  scheduler.Enqueue(&task4);

  EXPECT_EQ(scheduler.GetQueueSize(), 4, "Queue size should be 4");

  // 移除中间任务
  scheduler.Dequeue(&task2);
  EXPECT_EQ(scheduler.GetQueueSize(), 3,
            "Queue size should be 3 after dequeue");

  // 移除队首任务
  scheduler.Dequeue(&task1);
  EXPECT_EQ(scheduler.GetQueueSize(), 2,
            "Queue size should be 2 after dequeue");

  // 验证剩余任务顺序
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task3, "First remaining task should be task3");

  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task4, "Second remaining task should be task4");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_rr_dequeue passed");
  return true;
}

auto test_rr_statistics() -> bool {
  klog::Info("Running test_rr_statistics...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);

  // 初始统计
  auto stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 0, "Initial enqueues should be 0");
  EXPECT_EQ(stats.total_dequeues, 0, "Initial dequeues should be 0");
  EXPECT_EQ(stats.total_picks, 0, "Initial picks should be 0");
  EXPECT_EQ(stats.total_preemptions, 0, "Initial preemptions should be 0");

  // 测试入队统计
  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 2, "Enqueues should be 2");

  // 测试选择统计
  (void)scheduler.PickNext();
  (void)scheduler.PickNext();
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_picks, 2, "Picks should be 2");

  // 测试出队统计
  scheduler.Enqueue(&task1);
  scheduler.Dequeue(&task1);
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_dequeues, 1, "Dequeues should be 1");

  // 测试抢占统计
  scheduler.OnPreempted(&task1);
  scheduler.OnPreempted(&task2);
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_preemptions, 2, "Preemptions should be 2");

  // 重置统计
  scheduler.ResetStats();
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 0, "Enqueues should be 0 after reset");
  EXPECT_EQ(stats.total_dequeues, 0, "Dequeues should be 0 after reset");
  EXPECT_EQ(stats.total_picks, 0, "Picks should be 0 after reset");
  EXPECT_EQ(stats.total_preemptions, 0, "Preemptions should be 0 after reset");

  klog::Info("test_rr_statistics passed");
  return true;
}

auto test_rr_fairness() -> bool {
  klog::Info("Running test_rr_fairness...");

  RoundRobinScheduler scheduler;
  constexpr size_t kTaskCount = 50;
  TaskControlBlock* tasks[kTaskCount];

  // 初始化任务
  for (size_t i = 0; i < kTaskCount; ++i) {
    tasks[i] = new TaskControlBlock("Task", 10, nullptr, nullptr);
    tasks[i]->fsm.Receive(MsgSchedule{});
    scheduler.Enqueue(tasks[i]);
  }

  EXPECT_EQ(scheduler.GetQueueSize(), kTaskCount,
            "Queue size should match task count");

  // 验证所有任务按照加入顺序被选中
  for (size_t i = 0; i < kTaskCount; ++i) {
    auto* picked = scheduler.PickNext();
    EXPECT_NE(picked, nullptr, "Picked task should not be nullptr");
    EXPECT_EQ(picked, tasks[i], "Task should be picked in order");
  }

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty after all picks");

  // 清理内存
  for (size_t i = 0; i < kTaskCount; ++i) {
    delete tasks[i];
  }

  klog::Info("test_rr_fairness passed");
  return true;
}

auto test_rr_mixed_operations() -> bool {
  klog::Info("Running test_rr_mixed_operations...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});
  TaskControlBlock task4("Task4", 4, nullptr, nullptr);
  task4.fsm.Receive(MsgSchedule{});
  TaskControlBlock task5("Task5", 5, nullptr, nullptr);
  task5.fsm.Receive(MsgSchedule{});

  // 复杂的混合操作序列
  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);

  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task1, "First pick should be task1");

  scheduler.Enqueue(&task4);
  scheduler.Dequeue(&task3);
  scheduler.Enqueue(&task5);

  // 现在队列应该是 [task2, task4, task5]
  EXPECT_EQ(scheduler.GetQueueSize(), 3, "Queue size should be 3");

  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task2, "Second pick should be task2");

  auto* picked3 = scheduler.PickNext();
  EXPECT_EQ(picked3, &task4, "Third pick should be task4");

  auto* picked4 = scheduler.PickNext();
  EXPECT_EQ(picked4, &task5, "Fourth pick should be task5");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_rr_mixed_operations passed");
  return true;
}

auto test_rr_multiple_rounds() -> bool {
  klog::Info("Running test_rr_multiple_rounds...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});

  // 进行 5 轮时间片轮转
  for (int round = 0; round < 5; ++round) {
    scheduler.Enqueue(&task1);
    scheduler.Enqueue(&task2);
    scheduler.Enqueue(&task3);

    EXPECT_EQ(scheduler.PickNext(), &task1, "Round robin: task1");
    EXPECT_EQ(scheduler.PickNext(), &task2, "Round robin: task2");
    EXPECT_EQ(scheduler.PickNext(), &task3, "Round robin: task3");
    EXPECT_TRUE(scheduler.IsEmpty(), "Queue should be empty after each round");
  }

  klog::Info("test_rr_multiple_rounds passed");
  return true;
}

auto test_rr_hooks() -> bool {
  klog::Info("Running test_rr_hooks...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  task1.sched_info.priority = 5;

  // 测试各种钩子函数不会崩溃
  scheduler.OnScheduled(&task1);
  scheduler.OnPreempted(&task1);
  scheduler.BoostPriority(&task1, 10);
  scheduler.RestorePriority(&task1);

  // OnTick 应返回 false（RR 使用时间片而非 tick 计数）
  bool need_resched = scheduler.OnTick(&task1);
  EXPECT_EQ(need_resched, false, "OnTick should return false for RR");

  // OnTimeSliceExpired 应返回 true（需要重新入队）
  bool need_requeue = scheduler.OnTimeSliceExpired(&task1);
  EXPECT_EQ(need_requeue, true, "OnTimeSliceExpired should return true for RR");

  // 验证调度器仍正常工作
  scheduler.Enqueue(&task1);
  auto* picked = scheduler.PickNext();
  EXPECT_EQ(picked, &task1, "Scheduler should still work after hook calls");

  klog::Info("test_rr_hooks passed");
  return true;
}

auto test_rr_robustness() -> bool {
  klog::Info("Running test_rr_robustness...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});

  // 空队列操作
  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext on empty queue should return nullptr");
  scheduler.Dequeue(&task1);  // 不应崩溃

  // 重复移除
  scheduler.Enqueue(&task1);
  scheduler.Dequeue(&task1);
  scheduler.Dequeue(&task1);  // 再次移除已移除的任务，不应崩溃
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  // nullptr 处理
  scheduler.Enqueue(nullptr);  // 不应崩溃
  scheduler.Dequeue(nullptr);  // 不应崩溃
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should still be empty");

  klog::Info("test_rr_robustness passed");
  return true;
}

auto test_rr_interleaved_operations() -> bool {
  klog::Info("Running test_rr_interleaved_operations...");

  RoundRobinScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});
  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});

  // 交替的入队和出队操作
  scheduler.Enqueue(&task1);
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task1, "Should pick task1");

  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);
  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task2, "Should pick task2");

  scheduler.Enqueue(&task1);
  auto* picked3 = scheduler.PickNext();
  EXPECT_EQ(picked3, &task3, "Should pick task3");

  auto* picked4 = scheduler.PickNext();
  EXPECT_EQ(picked4, &task1, "Should pick task1 again");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_rr_interleaved_operations passed");
  return true;
}

}  // namespace

auto rr_scheduler_test() -> bool {
  klog::Info("\n=== Round-Robin Scheduler System Tests ===\n");

  if (!test_rr_basic_functionality()) {
    return false;
  }

  if (!test_rr_round_robin_behavior()) {
    return false;
  }

  if (!test_rr_time_slice_management()) {
    return false;
  }

  if (!test_rr_dequeue()) {
    return false;
  }

  if (!test_rr_statistics()) {
    return false;
  }

  if (!test_rr_fairness()) {
    return false;
  }

  if (!test_rr_mixed_operations()) {
    return false;
  }

  if (!test_rr_multiple_rounds()) {
    return false;
  }

  if (!test_rr_hooks()) {
    return false;
  }

  if (!test_rr_robustness()) {
    return false;
  }

  if (!test_rr_interleaved_operations()) {
    return false;
  }

  klog::Info("=== All Round-Robin Scheduler Tests Passed ===\n");
  return true;
}

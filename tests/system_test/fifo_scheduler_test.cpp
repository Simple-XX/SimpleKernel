/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "fifo_scheduler.hpp"

#include <cstdint>

#include "system_test.h"
#include "task_control_block.hpp"
#include "task_messages.hpp"

namespace {

auto test_fifo_basic_functionality() -> bool {
  klog::Info("Running test_fifo_basic_functionality...");

  FifoScheduler scheduler;

  // 验证调度器名称
  EXPECT_EQ(scheduler.name[0], 'F', "Scheduler name should start with F");

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

  // 验证 FIFO 顺序
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task1, "First picked task should be task1");

  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task2, "Second picked task should be task2");

  auto* picked3 = scheduler.PickNext();
  EXPECT_EQ(picked3, &task3, "Third picked task should be task3");

  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext should return nullptr after all tasks picked");

  klog::Info("test_fifo_basic_functionality passed");
  return true;
}

auto test_fifo_ordering() -> bool {
  klog::Info("Running test_fifo_ordering...");

  FifoScheduler scheduler;
  constexpr size_t kTaskCount = 10;
  TaskControlBlock* tasks[kTaskCount];

  // 初始化任务
  for (size_t i = 0; i < kTaskCount; ++i) {
    tasks[i] = new TaskControlBlock("Task", 10, nullptr, nullptr);
    tasks[i]->fsm.Receive(MsgSchedule{});
    scheduler.Enqueue(tasks[i]);
  }

  EXPECT_EQ(scheduler.GetQueueSize(), kTaskCount,
            "Queue size should match task count");

  // 验证严格的 FIFO 顺序
  for (size_t i = 0; i < kTaskCount; ++i) {
    auto* picked = scheduler.PickNext();
    EXPECT_NE(picked, nullptr, "Picked task should not be nullptr");
    EXPECT_EQ(picked, tasks[i], "Task should be picked in FIFO order");
  }

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty after all picks");

  // 清理内存
  for (size_t i = 0; i < kTaskCount; ++i) {
    delete tasks[i];
  }

  klog::Info("test_fifo_ordering passed");
  return true;
}

auto test_fifo_dequeue() -> bool {
  klog::Info("Running test_fifo_dequeue...");

  FifoScheduler scheduler;

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

  klog::Info("test_fifo_dequeue passed");
  return true;
}

auto test_fifo_statistics() -> bool {
  klog::Info("Running test_fifo_statistics...");

  FifoScheduler scheduler;

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

  klog::Info("test_fifo_statistics passed");
  return true;
}

auto test_fifo_mixed_operations() -> bool {
  klog::Info("Running test_fifo_mixed_operations...");

  FifoScheduler scheduler;

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

  klog::Info("test_fifo_mixed_operations passed");
  return true;
}

auto test_fifo_repeated_enqueue() -> bool {
  klog::Info("Running test_fifo_repeated_enqueue...");

  FifoScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});

  // 模拟任务多次时间片用完后重新入队
  scheduler.Enqueue(&task1);
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task1, "First pick should be task1");

  scheduler.Enqueue(&task1);
  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task1, "Second pick should be task1");

  scheduler.Enqueue(&task1);
  auto* picked3 = scheduler.PickNext();
  EXPECT_EQ(picked3, &task1, "Third pick should be task1");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_fifo_repeated_enqueue passed");
  return true;
}

auto test_fifo_hooks() -> bool {
  klog::Info("Running test_fifo_hooks...");

  FifoScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  task1.sched_info.priority = 5;

  // 测试各种钩子函数不会崩溃
  scheduler.OnScheduled(&task1);
  scheduler.OnPreempted(&task1);
  scheduler.BoostPriority(&task1, 10);
  scheduler.RestorePriority(&task1);

  // OnTick 应返回 false（FIFO 不需要基于 tick 的抢占）
  bool need_resched = scheduler.OnTick(&task1);
  EXPECT_EQ(need_resched, false, "OnTick should return false for FIFO");

  // OnTimeSliceExpired 应返回 true（需要重新入队）
  bool need_requeue = scheduler.OnTimeSliceExpired(&task1);
  EXPECT_EQ(need_requeue, true,
            "OnTimeSliceExpired should return true for FIFO");

  // 验证调度器仍正常工作
  scheduler.Enqueue(&task1);
  auto* picked = scheduler.PickNext();
  EXPECT_EQ(picked, &task1, "Scheduler should still work after hook calls");

  klog::Info("test_fifo_hooks passed");
  return true;
}

auto test_fifo_robustness() -> bool {
  klog::Info("Running test_fifo_robustness...");

  FifoScheduler scheduler;

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

  klog::Info("test_fifo_robustness passed");
  return true;
}

}  // namespace

auto fifo_scheduler_test() -> bool {
  klog::Info("\n=== FIFO Scheduler System Tests ===\n");

  if (!test_fifo_basic_functionality()) {
    return false;
  }

  if (!test_fifo_ordering()) {
    return false;
  }

  if (!test_fifo_dequeue()) {
    return false;
  }

  if (!test_fifo_statistics()) {
    return false;
  }

  if (!test_fifo_mixed_operations()) {
    return false;
  }

  if (!test_fifo_repeated_enqueue()) {
    return false;
  }

  if (!test_fifo_hooks()) {
    return false;
  }

  if (!test_fifo_robustness()) {
    return false;
  }

  klog::Info("=== All FIFO Scheduler Tests Passed ===\n");
  return true;
}

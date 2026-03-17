/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "cfs_scheduler.hpp"

#include <cstdint>

#include "system_test.h"
#include "task_control_block.hpp"
#include "task_messages.hpp"

namespace {

auto test_cfs_basic_functionality() -> bool {
  klog::Info("Running test_cfs_basic_functionality...");

  CfsScheduler scheduler;

  // 创建测试任务
  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.fsm.Receive(MsgSchedule{});
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 0;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.fsm.Receive(MsgSchedule{});
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 0;

  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.fsm.Receive(MsgSchedule{});
  task3.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task3.sched_data.cfs.vruntime = 0;

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

  // 选择任务
  auto* picked1 = scheduler.PickNext();
  EXPECT_NE(picked1, nullptr, "Picked task should not be nullptr");

  auto* picked2 = scheduler.PickNext();
  EXPECT_NE(picked2, nullptr, "Picked task should not be nullptr");

  auto* picked3 = scheduler.PickNext();
  EXPECT_NE(picked3, nullptr, "Picked task should not be nullptr");

  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext should return nullptr after all tasks picked");
  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_cfs_basic_functionality passed");
  return true;
}

auto test_cfs_vruntime_ordering() -> bool {
  klog::Info("Running test_cfs_vruntime_ordering...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 1000;  // 最大 vruntime

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 500;  // 最小 vruntime

  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task3.sched_data.cfs.vruntime = 750;  // 中间 vruntime

  // 按任意顺序加入任务
  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);

  EXPECT_EQ(scheduler.GetQueueSize(), 3, "Queue size should be 3");

  // 应该按 vruntime 从小到大选择
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task2, "First pick should be task2 (vruntime=500)");

  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task3, "Second pick should be task3 (vruntime=750)");

  auto* picked3 = scheduler.PickNext();
  EXPECT_EQ(picked3, &task1, "Third pick should be task1 (vruntime=1000)");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_cfs_vruntime_ordering passed");
  return true;
}

auto test_cfs_new_task_vruntime() -> bool {
  klog::Info("Running test_cfs_new_task_vruntime...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 1000;

  // 加入第一个任务并选择它
  scheduler.Enqueue(&task1);
  auto* picked = scheduler.PickNext();
  EXPECT_EQ(picked, &task1, "First pick should be task1");

  // 第二个新任务 (vruntime = 0)
  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 0;

  // 记录入队前的 vruntime
  uint64_t before_vruntime = task2.sched_data.cfs.vruntime;
  EXPECT_EQ(before_vruntime, 0, "New task vruntime should be 0 initially");

  scheduler.Enqueue(&task2);

  // 新任务的 vruntime 应该被设置为 min_vruntime
  EXPECT_NE(task2.sched_data.cfs.vruntime, 0,
            "New task vruntime should be initialized to min_vruntime");
  EXPECT_GE(task2.sched_data.cfs.vruntime, 1000,
            "New task vruntime should be >= min_vruntime");

  klog::Info("test_cfs_new_task_vruntime passed");
  return true;
}

auto test_cfs_weight_impact() -> bool {
  klog::Info("Running test_cfs_weight_impact...");

  CfsScheduler scheduler;

  TaskControlBlock high_priority("HighPriority", 1, nullptr, nullptr);
  high_priority.sched_data.cfs.weight =
      CfsScheduler::kDefaultWeight * 2;  // 2倍权重
  high_priority.sched_data.cfs.vruntime = 0;

  TaskControlBlock low_priority("LowPriority", 2, nullptr, nullptr);
  low_priority.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;  // 1倍权重
  low_priority.sched_data.cfs.vruntime = 0;

  // 模拟相同次数的 tick
  constexpr int kTickCount = 10;

  for (int i = 0; i < kTickCount; ++i) {
    (void)scheduler.OnTick(&high_priority);
    (void)scheduler.OnTick(&low_priority);
  }

  // 高优先级任务的 vruntime 增长应该慢于低优先级
  EXPECT_LT(high_priority.sched_data.cfs.vruntime,
            low_priority.sched_data.cfs.vruntime,
            "High priority task should have lower vruntime growth");

  uint64_t expected = high_priority.sched_data.cfs.vruntime * 2;
  uint64_t tolerance = expected / 10;
  EXPECT_GE(low_priority.sched_data.cfs.vruntime, expected - tolerance,
            "vruntime ratio lower bound (should be ~2x)");
  EXPECT_LE(low_priority.sched_data.cfs.vruntime, expected + tolerance,
            "vruntime ratio upper bound (should be ~2x)");

  klog::Info("test_cfs_weight_impact passed");
  return true;
}

auto test_cfs_preemption() -> bool {
  klog::Info("Running test_cfs_preemption...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 1000;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 0;  // 更小的 vruntime

  scheduler.Enqueue(&task2);

  // task1 运行时，应该被抢占
  bool should_preempt = scheduler.OnTick(&task1);
  EXPECT_TRUE(should_preempt, "Task with higher vruntime should be preempted");

  klog::Info("test_cfs_preemption passed");
  return true;
}

auto test_cfs_no_preemption() -> bool {
  klog::Info("Running test_cfs_no_preemption...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 1000;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  // OnTick 会让 task1 的 vruntime 增加 delta = (kDefaultWeight * 1000) / weight
  // task1 初始 vruntime=1000，OnTick 后 = 1000 + delta = 2000
  // 设置 task2 的 vruntime 使差距小于 kMinGranularity
  uint64_t delta =
      (CfsScheduler::kDefaultWeight * 1000) / task1.sched_data.cfs.weight;
  uint64_t task1_after_tick = task1.sched_data.cfs.vruntime + delta;
  task2.sched_data.cfs.vruntime =
      task1_after_tick - (CfsScheduler::kMinGranularity / 2);

  scheduler.Enqueue(&task2);

  // OnTick 后差距为 5，小于 kMinGranularity (10)，不应该抢占
  bool should_preempt = scheduler.OnTick(&task1);
  EXPECT_FALSE(
      should_preempt,
      "Task should not be preempted when vruntime difference is small");

  klog::Info("test_cfs_no_preemption passed");
  return true;
}

auto test_cfs_dequeue() -> bool {
  klog::Info("Running test_cfs_dequeue...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 100;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 200;

  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task3.sched_data.cfs.vruntime = 300;

  TaskControlBlock task4("Task4", 4, nullptr, nullptr);
  task4.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task4.sched_data.cfs.vruntime = 400;

  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);
  scheduler.Enqueue(&task4);

  EXPECT_EQ(scheduler.GetQueueSize(), 4, "Queue size should be 4");

  // 移除中间任务
  scheduler.Dequeue(&task2);
  EXPECT_EQ(scheduler.GetQueueSize(), 3,
            "Queue size should be 3 after dequeue");

  // 移除队首任务（vruntime 最小）
  scheduler.Dequeue(&task1);
  EXPECT_EQ(scheduler.GetQueueSize(), 2,
            "Queue size should be 2 after dequeue");

  // 验证剩余任务按 vruntime 顺序
  auto* picked1 = scheduler.PickNext();
  EXPECT_EQ(picked1, &task3, "First remaining task should be task3");

  auto* picked2 = scheduler.PickNext();
  EXPECT_EQ(picked2, &task4, "Second remaining task should be task4");

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_cfs_dequeue passed");
  return true;
}

auto test_cfs_statistics() -> bool {
  klog::Info("Running test_cfs_statistics...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 0;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 0;

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

  // 重置统计
  scheduler.ResetStats();
  stats = scheduler.GetStats();
  EXPECT_EQ(stats.total_enqueues, 0, "Enqueues should be 0 after reset");
  EXPECT_EQ(stats.total_dequeues, 0, "Dequeues should be 0 after reset");
  EXPECT_EQ(stats.total_picks, 0, "Picks should be 0 after reset");
  EXPECT_EQ(stats.total_preemptions, 0, "Preemptions should be 0 after reset");

  klog::Info("test_cfs_statistics passed");
  return true;
}

auto test_cfs_min_vruntime_update() -> bool {
  klog::Info("Running test_cfs_min_vruntime_update...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 1000;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task2.sched_data.cfs.vruntime = 500;

  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task3.sched_data.cfs.vruntime = 750;

  // 初始 min_vruntime 应该是 0
  EXPECT_EQ(scheduler.GetMinVruntime(), 0, "Initial min_vruntime should be 0");

  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);

  // 选择 task2 (vruntime = 500)
  (void)scheduler.PickNext();

  // min_vruntime 应该更新为队列中最小的 (750)
  uint64_t min_vruntime = scheduler.GetMinVruntime();
  EXPECT_GE(min_vruntime, 500, "min_vruntime should be updated");

  klog::Info("test_cfs_min_vruntime_update passed");
  return true;
}

auto test_cfs_multiple_ticks() -> bool {
  klog::Info("Running test_cfs_multiple_ticks...");

  CfsScheduler scheduler;

  TaskControlBlock task("Task", 1, nullptr, nullptr);
  task.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task.sched_data.cfs.vruntime = 0;

  uint64_t initial_vruntime = task.sched_data.cfs.vruntime;

  // 模拟多次 tick
  constexpr int kTickCount = 10;
  for (int i = 0; i < kTickCount; ++i) {
    (void)scheduler.OnTick(&task);
  }

  // vruntime 应该累积增长
  EXPECT_GT(task.sched_data.cfs.vruntime, initial_vruntime,
            "vruntime should accumulate over ticks");

  // 计算预期的增长
  uint64_t expected_delta =
      (CfsScheduler::kDefaultWeight * 1000) / task.sched_data.cfs.weight;
  uint64_t expected_vruntime = initial_vruntime + expected_delta * kTickCount;
  EXPECT_EQ(task.sched_data.cfs.vruntime, expected_vruntime,
            "vruntime should grow by expected amount");

  klog::Info("test_cfs_multiple_ticks passed");
  return true;
}

auto test_cfs_fairness() -> bool {
  klog::Info("Running test_cfs_fairness...");

  CfsScheduler scheduler;

  // 创建三个相同权重的任务
  constexpr size_t kTaskCount = 3;
  TaskControlBlock* tasks[kTaskCount];

  for (size_t i = 0; i < kTaskCount; ++i) {
    tasks[i] = new TaskControlBlock("Task", 10, nullptr, nullptr);
    tasks[i]->sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
    tasks[i]->sched_data.cfs.vruntime = 0;
    scheduler.Enqueue(tasks[i]);
  }

  // 模拟多轮调度
  constexpr int kRounds = 5;
  for (int round = 0; round < kRounds; ++round) {
    // 每轮选择所有任务
    for (size_t i = 0; i < kTaskCount; ++i) {
      auto* task = scheduler.PickNext();
      EXPECT_NE(task, nullptr, "Should pick a task");

      // 模拟任务运行一段时间
      for (int tick = 0; tick < 5; ++tick) {
        (void)scheduler.OnTick(task);
      }

      // 将任务重新入队
      scheduler.Enqueue(task);
    }
  }

  // 检查所有任务的 vruntime 应该相近（公平性）
  uint64_t max_vruntime = 0;
  uint64_t min_vruntime = UINT64_MAX;

  for (size_t i = 0; i < kTaskCount; ++i) {
    if (tasks[i]->sched_data.cfs.vruntime > max_vruntime) {
      max_vruntime = tasks[i]->sched_data.cfs.vruntime;
    }
    if (tasks[i]->sched_data.cfs.vruntime < min_vruntime) {
      min_vruntime = tasks[i]->sched_data.cfs.vruntime;
    }
  }

  // vruntime 差异应该较小（表示公平性）
  uint64_t difference = max_vruntime - min_vruntime;
  EXPECT_LT(difference, 10000,
            "vruntime difference should be small (fairness)");

  // 清理内存
  for (size_t i = 0; i < kTaskCount; ++i) {
    delete tasks[i];
  }

  klog::Info("test_cfs_fairness passed");
  return true;
}

auto test_cfs_mixed_operations() -> bool {
  klog::Info("Running test_cfs_mixed_operations...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 100;

  TaskControlBlock task2("Task2", 2, nullptr, nullptr);
  task2.sched_data.cfs.weight = CfsScheduler::kDefaultWeight * 2;
  task2.sched_data.cfs.vruntime = 200;

  TaskControlBlock task3("Task3", 3, nullptr, nullptr);
  task3.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task3.sched_data.cfs.vruntime = 300;

  TaskControlBlock task4("Task4", 4, nullptr, nullptr);
  task4.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task4.sched_data.cfs.vruntime = 0;

  TaskControlBlock task5("Task5", 5, nullptr, nullptr);
  task5.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task5.sched_data.cfs.vruntime = 0;

  // 复杂的混合操作序列
  scheduler.Enqueue(&task1);
  scheduler.Enqueue(&task2);
  scheduler.Enqueue(&task3);

  auto* picked1 = scheduler.PickNext();  // 应该是 task1 (vruntime=100)
  EXPECT_EQ(picked1, &task1, "First pick should be task1");

  scheduler.Enqueue(&task4);  // 新任务
  scheduler.Dequeue(&task3);
  scheduler.Enqueue(&task5);  // 新任务

  EXPECT_EQ(scheduler.GetQueueSize(), 3,
            "Queue size should be 3 after operations");

  // 继续选择
  auto* picked2 = scheduler.PickNext();
  EXPECT_NE(picked2, nullptr, "Second pick should not be nullptr");

  klog::Info("test_cfs_mixed_operations passed");
  return true;
}

auto test_cfs_robustness() -> bool {
  klog::Info("Running test_cfs_robustness...");

  CfsScheduler scheduler;

  TaskControlBlock task1("Task1", 1, nullptr, nullptr);
  task1.sched_data.cfs.weight = CfsScheduler::kDefaultWeight;
  task1.sched_data.cfs.vruntime = 0;

  // 空队列操作
  EXPECT_EQ(scheduler.PickNext(), nullptr,
            "PickNext on empty queue should return nullptr");
  scheduler.Dequeue(&task1);  // 不应崩溃

  // nullptr 操作
  scheduler.Enqueue(nullptr);
  scheduler.Dequeue(nullptr);
  scheduler.OnPreempted(nullptr);
  scheduler.OnScheduled(nullptr);

  // 权重为 0 的任务
  task1.sched_data.cfs.weight = 0;
  scheduler.Enqueue(&task1);
  EXPECT_NE(task1.sched_data.cfs.weight, 0,
            "Weight should be set to default if 0");

  // 重复移除
  scheduler.Dequeue(&task1);
  scheduler.Dequeue(&task1);  // 不应崩溃

  EXPECT_TRUE(scheduler.IsEmpty(), "Scheduler should be empty");

  klog::Info("test_cfs_robustness passed");
  return true;
}

}  // namespace

auto cfs_scheduler_test() -> bool {
  klog::Info("\n=== CFS Scheduler System Tests ===\n");

  if (!test_cfs_basic_functionality()) {
    return false;
  }

  if (!test_cfs_vruntime_ordering()) {
    return false;
  }

  if (!test_cfs_new_task_vruntime()) {
    return false;
  }

  if (!test_cfs_weight_impact()) {
    return false;
  }

  if (!test_cfs_preemption()) {
    return false;
  }

  if (!test_cfs_no_preemption()) {
    return false;
  }

  if (!test_cfs_dequeue()) {
    return false;
  }

  if (!test_cfs_statistics()) {
    return false;
  }

  if (!test_cfs_min_vruntime_update()) {
    return false;
  }

  if (!test_cfs_multiple_ticks()) {
    return false;
  }

  if (!test_cfs_fairness()) {
    return false;
  }

  if (!test_cfs_mixed_operations()) {
    return false;
  }

  if (!test_cfs_robustness()) {
    return false;
  }

  klog::Info("=== All CFS Scheduler Tests Passed ===\n");
  return true;
}

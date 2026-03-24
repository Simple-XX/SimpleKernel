/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/list.h>

#include "kernel_config.hpp"
#include "kernel_log.hpp"
#include "scheduler_base.hpp"
#include "task_control_block.hpp"

/**
 * @brief Round-Robin 调度器
 *
 * 时间片轮转调度器，所有任务按照 FIFO 顺序排队。
 * 每个任务获得相同的时间片，时间片用完后放回队列尾部。
 */
class RoundRobinScheduler : public SchedulerBase {
 public:
  /**
   * @brief 将任务加入就绪队列尾部
   * @param task 任务控制块指针
   *
   * 重置任务的时间片并将其加入队列尾部，实现公平的时间片轮转。
   */
  auto Enqueue(TaskControlBlock* task) -> void override {
    if (task) {
      if (ready_queue_.full()) {
        klog::Err(
            "RoundRobinScheduler::Enqueue: ready_queue full, dropping task");
        return;
      }
      // 重新分配时间片
      task->sched_info.time_slice_remaining =
          task->sched_info.time_slice_default;
      ready_queue_.push_back(task);
      stats_.total_enqueues++;
    }
  }

  /**
   * @brief 从就绪队列中移除指定任务
   * @param task 要移除的任务控制块指针
   *
   * 用于任务主动退出或被阻塞等场景。
   */
  auto Dequeue(TaskControlBlock* task) -> void override {
    if (!task) {
      return;
    }

    for (auto it = ready_queue_.begin(); it != ready_queue_.end(); ++it) {
      if (*it == task) {
        ready_queue_.erase(it);
        stats_.total_dequeues++;
        break;
      }
    }
  }

  /**
   * @brief 选择下一个要运行的任务
   * @return TaskControlBlock* 下一个任务，如果队列为空则返回 nullptr
   *
   * 从队列头部取出任务，实现 Round-Robin 轮转。
   */
  [[nodiscard]] auto PickNext() -> TaskControlBlock* override {
    if (ready_queue_.empty()) {
      return nullptr;
    }
    auto next = ready_queue_.front();
    ready_queue_.pop_front();
    stats_.total_picks++;
    return next;
  }

  /**
   * @brief 获取就绪队列大小
   * @return size_t 队列中的任务数量
   */
  [[nodiscard]] auto GetQueueSize() const -> size_t override {
    return ready_queue_.size();
  }

  /**
   * @brief 判断队列是否为空
   * @return bool 队列为空返回 true
   */
  [[nodiscard]] auto IsEmpty() const -> bool override {
    return ready_queue_.empty();
  }

  /**
   * @brief 时间片耗尽处理
   * @param task 时间片耗尽的任务
   * @return bool 返回 true 表示需要重新入队
   *
   * Round-Robin 调度器在时间片耗尽时重置时间片并将任务放回队列尾部。
   */
  [[nodiscard]] auto OnTimeSliceExpired(TaskControlBlock* task)
      -> bool override {
    if (task) {
      // 重新分配时间片
      task->sched_info.time_slice_remaining =
          task->sched_info.time_slice_default;
    }
    return true;
  }

  /**
   * @brief 任务被抢占时调用
   * @param task 被抢占的任务
   */
  auto OnPreempted([[maybe_unused]] TaskControlBlock* task) -> void override {
    stats_.total_preemptions++;
  }

  /// @name 构造/析构函数
  /// @{
  RoundRobinScheduler() = default;
  RoundRobinScheduler(const RoundRobinScheduler&) = delete;
  RoundRobinScheduler(RoundRobinScheduler&&) = delete;
  auto operator=(const RoundRobinScheduler&) -> RoundRobinScheduler& = delete;
  auto operator=(RoundRobinScheduler&&) -> RoundRobinScheduler& = delete;
  ~RoundRobinScheduler() override = default;
  /// @}

 private:
  /// 就绪队列 (双向链表，支持从头部取、向尾部放，固定容量)
  etl::list<TaskControlBlock*, kernel::config::kMaxReadyTasks> ready_queue_;
};

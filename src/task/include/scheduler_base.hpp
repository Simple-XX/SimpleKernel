/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "task_control_block.hpp"

/**
 * @brief 新任务首次执行时的引导函数 (由 kernel_thread_entry 汇编调用)
 *
 * @param entry 任务入口函数
 * @param arg   传递给入口函数的参数
 *
 * @pre  由 kernel_thread_entry 汇编以 C 调用约定调用
 * @post 中断已开启，任务入口函数已执行，进程已退出
 */
extern "C" [[noreturn]] void kernel_thread_bootstrap(void (*entry)(void*),
                                                     void* arg);

/**
 * @brief 调度器基类接口
 *
 * 提供统一的调度器接口，支持多种调度策略 (FIFO, Round-Robin, CFS, MLFQ 等)
 * 调度器负责管理就绪队列，选择下一个要运行的任务，以及处理调度相关事件
 */
class SchedulerBase {
 public:
  /// 调度器名称
  const char* name{"Unnamed Scheduler"};

  /**
   * @brief 调度器统计信息
   */
  struct Stats {
    /// 总入队次数
    size_t total_enqueues{0};
    /// 总出队次数
    size_t total_dequeues{0};
    /// 总选择次数
    size_t total_picks{0};
    /// 总抢占次数
    size_t total_preemptions{0};
  };

  /**
   * @brief 将任务加入就绪队列
   * @param task 要加入的任务
   */
  virtual auto Enqueue(TaskControlBlock* task) -> void = 0;

  /**
   * @brief 从就绪队列移除指定任务 (用于任务阻塞/退出)
   * @param task 要移除的任务
   */
  virtual auto Dequeue(TaskControlBlock* task) -> void = 0;

  /**
   * @brief 选择下一个要运行的任务 (不会从队列中移除)
   * @return 下一个要运行的任务，如果队列为空则返回 nullptr
   */
  [[nodiscard]] virtual auto PickNext() -> TaskControlBlock* = 0;

  /**
   * @brief 获取就绪队列中的任务数量 (用于负载均衡)
   * @return 队列中的任务数量
   */
  [[nodiscard]] virtual auto GetQueueSize() const -> size_t = 0;

  /**
   * @brief 判断队列是否为空
   * @return 队列为空返回 true，否则返回 false
   */
  [[nodiscard]] virtual auto IsEmpty() const -> bool = 0;

  /**
   * @brief Tick 更新：每个时钟中断时调用，用于更新调度器状态
   *
   * @param current 当前正在运行的任务
   * @return true 表示需要重新调度
   * @return false 表示继续运行当前任务
   */
  [[nodiscard]] virtual auto OnTick([[maybe_unused]] TaskControlBlock* current)
      -> bool {
    return false;
  }

  /**
   * @brief 时间片耗尽处理：当任务时间片用完时调用
   *
   * 用于支持自定义时间片处理（如 MLFQ 的优先级降级）
   *
   * @param task 时间片耗尽的任务
   * @return true 表示任务需要重新入队
   * @return false 表示任务不需要重新入队
   */
  [[nodiscard]] virtual auto OnTimeSliceExpired(
      [[maybe_unused]] TaskControlBlock* task) -> bool {
    return true;
  }

  /**
   * @brief 优先级提升：当任务持有资源时被更高优先级任务等待，提升其优先级
   *
   * 用于支持优先级继承协议（防止优先级反转）
   *
   * @param task 需要提升优先级的任务
   * @param new_priority 新的优先级（继承自等待者中的最高优先级）
   */
  virtual auto BoostPriority([[maybe_unused]] TaskControlBlock* task,
                             [[maybe_unused]] int new_priority) -> void {}

  /**
   * @brief 优先级恢复：当任务释放资源后，恢复其原始优先级
   *
   * @param task 需要恢复优先级的任务
   */
  virtual auto RestorePriority([[maybe_unused]] TaskControlBlock* task)
      -> void {}

  /**
   * @brief 任务被抢占时调用 (从 Running 变为 Ready)
   *
   * 用于支持状态跟踪和统计
   *
   * @param task 被抢占的任务
   */
  virtual auto OnPreempted([[maybe_unused]] TaskControlBlock* task) -> void {}

  /**
   * @brief 任务开始运行时调用 (从 Ready 变为 Running)
   *
   * @param task 即将运行的任务
   */
  virtual auto OnScheduled([[maybe_unused]] TaskControlBlock* task) -> void {}

  /**
   * @brief 获取调度器统计信息
   * @return 统计信息结构体
   */
  [[nodiscard]] virtual auto GetStats() const -> const Stats& { return stats_; }

  /**
   * @brief 重置统计信息
   */
  virtual auto ResetStats() -> void { stats_ = {}; }

  /// @name 构造/析构函数
  /// @{
  SchedulerBase() = default;
  SchedulerBase(const SchedulerBase&) = default;
  SchedulerBase(SchedulerBase&&) = default;
  auto operator=(const SchedulerBase&) -> SchedulerBase& = default;
  auto operator=(SchedulerBase&&) -> SchedulerBase& = default;
  virtual ~SchedulerBase() = default;
  /// @}

 protected:
  Stats stats_{};
};

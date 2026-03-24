/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>

#include "project_config.h"

namespace kernel::config {

/// 全局最大任务数（task_table_ 容量）
inline constexpr size_t kMaxTasks = 256;
/// task_table_ 桶数
inline constexpr size_t kMaxTasksBuckets = 2 * kMaxTasks;

/// 每个 CPU 的最大睡眠任务数（sleeping_tasks 容量）
inline constexpr size_t kMaxSleepingTasks = 128;

/// 阻塞队列：最大资源组数（blocked_tasks 的 map 容量）
inline constexpr size_t kMaxBlockedGroups = 64;
/// 阻塞队列：map 桶数
inline constexpr size_t kMaxBlockedGroupsBuckets = 2 * kMaxBlockedGroups;
/// 阻塞队列：每组最大阻塞任务数（etl::list 容量）
inline constexpr size_t kMaxBlockedPerGroup = 32;

/// 调度器就绪队列容量（FIFO / RR / CFS）
inline constexpr size_t kMaxReadyTasks = 128;

/// 最大中断线程数
inline constexpr size_t kMaxInterruptThreads = 32;
/// 中断线程 map 桶数
inline constexpr size_t kMaxInterruptThreadsBuckets = 2 * kMaxInterruptThreads;

/// 最大 tick 观察者数
inline constexpr size_t kTickObservers = 8;
/// 最大 panic 观察者数
inline constexpr size_t kPanicObservers = 4;
}  // namespace kernel::config

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/singleton.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

struct TaskControlBlock;
struct CpuSchedData;

namespace per_cpu {

/// TimerHandler 重入保护（per-CPU，防止嵌套 timer 中断导致递归调度）
inline bool in_timer_handler[SIMPLEKERNEL_MAX_CORE_COUNT]{};

/// @brief 每个 CPU 核心的局部数据
struct PerCpu {
  /// 核心 ID
  size_t core_id{0};

  /// 当前运行的任务
  TaskControlBlock* running_task{nullptr};
  /// 空闲任务
  TaskControlBlock* idle_task{nullptr};
  /// 调度数据 (RunQueue) 指针
  CpuSchedData* sched_data{nullptr};

  /// @name 构造/析构函数
  /// @{
  explicit PerCpu(size_t id) : core_id(id) {}

  PerCpu() = default;
  PerCpu(const PerCpu&) = default;
  PerCpu(PerCpu&&) = default;
  auto operator=(const PerCpu&) -> PerCpu& = default;
  auto operator=(PerCpu&&) -> PerCpu& = default;
  ~PerCpu() = default;
  /// @}
} __attribute__((aligned(SIMPLEKERNEL_PER_CPU_ALIGN_SIZE)));

static_assert(sizeof(PerCpu) <= SIMPLEKERNEL_PER_CPU_ALIGN_SIZE,
              "PerCpu size should not exceed cache line size");

/// PerCpu 数组单例类型
using PerCpuArraySingleton =
    etl::singleton<std::array<PerCpu, SIMPLEKERNEL_MAX_CORE_COUNT>>;

/// @brief 获取当前核心的 PerCpu 数据
static __always_inline auto GetCurrentCore() -> PerCpu& {
  return PerCpuArraySingleton::instance()[cpu_io::GetCurrentCoreId()];
}

}  // namespace per_cpu

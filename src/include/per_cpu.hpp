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

/**
 * @brief 抢占/中断嵌套状态（per-CPU）
 * @note 只能在关中断或持有 per-CPU 数据时访问，不需要原子操作
 */
struct PreemptState {
  /// 硬中断嵌套深度（替代原 in_timer_handler 布尔标志）
  uint32_t hardirq_count{0};
  /// 软中断嵌套深度（预留，当前未使用）
  uint32_t softirq_count{0};
  /// 显式抢占禁用深度
  uint32_t preempt_disable_count{0};

  /// 延迟调度标志：由 TickUpdate 在中断上下文中设置，
  /// 在 TimerHandler 退出硬中断后检查并调用 Schedule()
  bool need_resched{false};

  /// 延迟负载均衡标志：由 TickUpdate 在中断上下文中设置，
  /// 在 TimerHandler 退出硬中断后检查并调用 Balance()
  bool need_balance{false};

  /// 是否处于中断上下文（硬中断或软中断）
  [[nodiscard]] auto InInterrupt() const -> bool {
    return hardirq_count > 0 || softirq_count > 0;
  }

  /// 是否处于硬中断上下文
  [[nodiscard]] auto InHardIrq() const -> bool { return hardirq_count > 0; }

  /// 当前是否可被抢占
  /// @details 仅当不在中断上下文且抢占未被显式禁用时可抢占
  [[nodiscard]] auto Preemptible() const -> bool {
    return hardirq_count == 0 && softirq_count == 0 &&
           preempt_disable_count == 0;
  }
};

/// 每个 CPU 核心的局部数据
struct PerCpu {
  /// 核心 ID
  size_t core_id{0};

  /// 当前运行的任务
  TaskControlBlock* running_task{nullptr};
  /// 空闲任务
  TaskControlBlock* idle_task{nullptr};
  /// 调度数据 (RunQueue) 指针
  CpuSchedData* sched_data{nullptr};

  /// 抢占/中断嵌套状态
  PreemptState preempt{};

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

/// 获取当前核心的 PerCpu 数据
static __always_inline auto GetCurrentCore() -> PerCpu& {
  return PerCpuArraySingleton::instance()[cpu_io::GetCurrentCoreId()];
}

}  // namespace per_cpu

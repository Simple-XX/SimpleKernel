/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/singleton.h>

#include <cstdint>

#include "gic.h"
#include "interrupt_base.h"
#include "sk_stdio.h"

/**
 * @brief AArch64 中断控制器实现
 */
class Interrupt final : public InterruptBase {
 public:
  /// 最大中断号
  static constexpr size_t kMaxInterrupt = 128;

  /**
   * @brief 执行中断处理
   * @param cause 中断号
   * @param context 中断上下文
   */
  auto Do(uint64_t cause, cpu_io::TrapContext* context) -> void override;

  /**
   * @brief 注册中断处理函数
   * @param cause 中断号
   * @param func 中断处理委托
   */
  auto RegisterInterruptFunc(uint64_t cause, InterruptDelegate func)
      -> void override;

  /**
   * @brief 发送 IPI 到指定 CPU
   * @param target_cpu_mask 目标 CPU 位掩码
   * @return Expected<void> 成功或失败
   */
  [[nodiscard]] auto SendIpi(uint64_t target_cpu_mask)
      -> Expected<void> override;

  /**
   * @brief 广播 IPI 到所有 CPU
   * @return Expected<void> 成功或失败
   */
  [[nodiscard]] auto BroadcastIpi() -> Expected<void> override;

  /**
   * @brief 注册外部中断
   * @param irq GIC INTID（已含 kSpiBase 偏移）
   * @param cpu_id CPU 编号
   * @param priority 中断优先级
   * @param handler 中断处理委托
   * @return Expected<void> 成功或失败
   */
  [[nodiscard]] auto RegisterExternalInterrupt(uint32_t irq, uint32_t cpu_id,
                                               uint32_t priority,
                                               InterruptDelegate handler)
      -> Expected<void> override;

  /**
   * @brief 设置 GIC
   */
  __always_inline auto SetUp() const -> void { gic_.SetUp(); }

  /**
   * @brief 设置 SPI 中断
   * @param intid 中断号
   * @param cpuid CPU 编号
   */
  __always_inline auto Spi(uint32_t intid, uint32_t cpuid) const -> void {
    gic_.Spi(intid, cpuid);
  }

  /**
   * @brief 设置 PPI 中断
   * @param intid 中断号
   * @param cpuid CPU 编号
   */
  __always_inline auto Ppi(uint32_t intid, uint32_t cpuid) const -> void {
    gic_.Ppi(intid, cpuid);
  }

  /**
   * @brief 设置 SGI 中断
   * @param intid 中断号
   * @param cpuid CPU 编号
   */
  __always_inline auto Sgi(uint32_t intid, uint32_t cpuid) const -> void {
    gic_.Sgi(intid, cpuid);
  }

  /// @name 构造/析构函数
  /// @{
  Interrupt();
  Interrupt(const Interrupt&) = delete;
  Interrupt(Interrupt&&) = delete;
  auto operator=(const Interrupt&) -> Interrupt& = delete;
  auto operator=(Interrupt&&) -> Interrupt& = delete;
  ~Interrupt() = default;
  /// @}

 private:
  /// 中断处理函数数组
  std::array<InterruptDelegate, kMaxInterrupt> interrupt_handlers_{};

  /// GIC 中断控制器实例
  Gic gic_{};
};

/// Interrupt 单例类型别名
using InterruptSingleton = etl::singleton<Interrupt>;

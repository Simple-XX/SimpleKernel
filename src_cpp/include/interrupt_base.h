/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/delegate.h>

#include <cstdint>

#include "expected.hpp"

/**
 * @brief 中断子系统抽象基类
 *
 * 所有架构的中断处理必须实现此接口。
 * 已知实现：PLIC（RISC-V）、GIC（AArch64）
 *
 * @pre  硬件中断控制器已初始化
 * @post 可通过 RegisterInterruptFunc 注册中断处理函数
 */
class InterruptBase {
 public:
  /**
   * @brief 类型安全的中断处理委托
   * @param  cause 中断或异常号
   * @param  context 中断上下文
   * @return uint64_t 返回值，0 成功
   */
  using InterruptDelegate =
      etl::delegate<uint64_t(uint64_t, cpu_io::TrapContext*)>;

  /// @name 构造/析构函数
  /// @{
  InterruptBase() = default;
  InterruptBase(const InterruptBase&) = delete;
  InterruptBase(InterruptBase&&) = delete;
  auto operator=(const InterruptBase&) -> InterruptBase& = delete;
  auto operator=(InterruptBase&&) -> InterruptBase& = delete;
  virtual ~InterruptBase() = default;
  /// @}

  /**
   * @brief 执行中断处理
   * @param 不同平台有不同含义
   */
  virtual auto Do(uint64_t, cpu_io::TrapContext*) -> void = 0;

  /**
   * @brief 注册中断处理函数
   * @param cause 中断号，不同平台有不同含义
   * @param func 处理函数
   */
  virtual auto RegisterInterruptFunc(uint64_t cause, InterruptDelegate func)
      -> void = 0;

  /**
   * @brief 发送 IPI 到指定核心
   * @param target_cpu_mask 目标核心的位掩码
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] virtual auto SendIpi(uint64_t target_cpu_mask)
      -> Expected<void> = 0;

  /**
   * @brief 广播 IPI 到所有其他核心
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] virtual auto BroadcastIpi() -> Expected<void> = 0;

  /**
   * @brief 注册外部中断处理函数
   * @param irq 外部中断号（平台相关: PLIC source_id / GIC INTID）
   * @param cpu_id 目标 CPU 核心 ID，中断将路由到该核心
   * @param priority 中断优先级
   * @param handler 中断处理函数
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] virtual auto RegisterExternalInterrupt(
      uint32_t irq, uint32_t cpu_id, uint32_t priority,
      InterruptDelegate handler) -> Expected<void> = 0;
};

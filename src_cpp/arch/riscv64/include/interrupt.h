/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/singleton.h>

#include <array>
#include <cstdint>

#include "interrupt_base.h"
#include "plic.h"
#include "sk_stdio.h"

class Interrupt final : public InterruptBase {
 public:
  /**
   * @brief 执行中断处理
   * @param cause 中断原因值（scause 寄存器内容）
   * @param context 中断上下文
   */
  auto Do(uint64_t cause, cpu_io::TrapContext* context) -> void override;
  /**
   * @brief 注册中断处理函数
   * @param cause 中断原因值（scause 格式）
   * @param func 中断处理委托
   */
  auto RegisterInterruptFunc(uint64_t cause, InterruptDelegate func)
      -> void override;
  /**
   * @brief 发送 IPI 到指定核心
   * @param target_cpu_mask 目标核心的位掩码
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] auto SendIpi(uint64_t target_cpu_mask)
      -> Expected<void> override;
  /**
   * @brief 广播 IPI 到所有其他核心
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] auto BroadcastIpi() -> Expected<void> override;
  /**
   * @brief 注册外部中断处理函数
   * @param irq 外部中断号（PLIC source_id）
   * @param cpu_id 目标 CPU 核心 ID
   * @param priority 中断优先级
   * @param handler 中断处理委托
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] auto RegisterExternalInterrupt(uint32_t irq, uint32_t cpu_id,
                                               uint32_t priority,
                                               InterruptDelegate handler)
      -> Expected<void> override;

  /// @name PLIC 访问接口
  /// @{
  /**
   * @brief 获取 PLIC 引用
   * @return Plic& PLIC 实例引用
   */
  [[nodiscard]] __always_inline auto plic() -> Plic& { return plic_; }
  /**
   * @brief 获取 PLIC 的只读引用
   * @return const Plic& PLIC 实例常量引用
   */
  [[nodiscard]] __always_inline auto plic() const -> const Plic& {
    return plic_;
  }

  /**
   * @brief 初始化 PLIC
   * @param dev_addr PLIC 设备地址
   * @param ndev 支持的中断源数量
   * @param context_count 上下文数量
   */
  auto InitPlic(uint64_t dev_addr, size_t ndev, size_t context_count) -> void;
  /// @}

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
  std::array<InterruptDelegate, cpu_io::ScauseInfo::kInterruptMaxCount>
      interrupt_handlers_{};
  /// 异常处理函数数组
  std::array<InterruptDelegate, cpu_io::ScauseInfo::kExceptionMaxCount>
      exception_handlers_{};
  Plic plic_{};
};

using InterruptSingleton = etl::singleton<Interrupt>;

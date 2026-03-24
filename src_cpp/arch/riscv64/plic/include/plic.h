/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

#include "interrupt_base.h"

/**
 * @brief Plic 驱动
 * @see https://github.com/riscv/riscv-plic
 */
class Plic {
 public:
  using InterruptDelegate = InterruptBase::InterruptDelegate;

  /// 最大外部中断数量
  static constexpr size_t kInterruptMaxCount = 16;

  /**
   * @brief 向 Plic 询问中断
   * @return uint32_t 中断源 ID (1-1023)
   */
  [[nodiscard]] auto Which() const -> uint32_t;

  /**
   * @brief 告知 Plic 已经处理了当前 IRQ
   * @param  source_id 中断源 ID (1-1023)
   */
  auto Done(uint32_t source_id) const -> void;

  /**
   * @brief 设置指定中断源的使能状态
   * @param hart_id hart ID
   * @param source_id 中断源 ID (1-1023)
   * @param priority 中断优先级 (0-7, 0 表示禁用)
   * @param enable 是否使能该中断
   */
  auto Set(uint32_t hart_id, uint32_t source_id, uint32_t priority,
           bool enable) const -> void;

  /**
   * @brief 获取指定中断源的状态信息
   * @param hart_id hart ID
   * @param source_id 中断源 ID (1-1023)
   * @return <优先级, 使能状态, 挂起状态>
   */
  [[nodiscard]] auto Get(uint32_t hart_id, uint32_t source_id) const
      -> std::tuple<uint32_t, bool, bool>;

  /**
   * @brief 注册外部中断处理函数
   * @param  cause             外部中断号
   * @param  func 外部中断处理函数
   */
  auto RegisterInterruptFunc(uint8_t cause, InterruptDelegate func) -> void;

  /**
   * @brief 执行外部中断处理
   * @param  cause              外部中断号
   * @param  context 中断上下文
   */
  auto Do(uint64_t cause, cpu_io::TrapContext* context) -> void;

  /// @name 构造/析构函数
  /// @{

  /**
   * @brief 构造函数
   * @param dev_addr 设备地址
   * @param ndev 支持的中断源数量 (riscv,ndev)
   * @param context_count 上下文数量 (通常为 2 * core_count)
   */
  explicit Plic(uint64_t dev_addr, size_t ndev, size_t context_count);

  Plic() = default;
  Plic(const Plic&) = default;
  Plic(Plic&&) = default;
  auto operator=(const Plic&) -> Plic& = default;
  auto operator=(Plic&&) -> Plic& = default;
  ~Plic() = default;
  /// @}

 private:
  static constexpr uint64_t kSourcePriorityOffset = 0x000000;
  static constexpr uint64_t kPendingBitsOffset = 0x001000;
  static constexpr uint64_t kEnableBitsOffset = 0x002000;
  static constexpr uint64_t kContextOffset = 0x200000;

  // 每个 context 的大小和偏移
  static constexpr uint64_t kContextSize = 0x1000;
  static constexpr uint64_t kPriorityThresholdOffset = 0x0;
  static constexpr uint64_t kClaimCompleteOffset = 0x4;

  // Enable bits 每个 context 的大小 (最多支持 1024 个中断源)
  static constexpr uint64_t kEnableSize = 0x80;

  /// 外部中断处理函数数组
  static std::array<InterruptDelegate, kInterruptMaxCount> interrupt_handlers_;

  uint64_t base_addr_{0};
  size_t ndev_{0};
  size_t context_count_{0};

  /**
   * @brief 计算 context ID
   * @param hart_id hart ID
   * @param mode 模式 (0=M-mode, 1=S-mode)
   * @return uint32_t context ID
   * @note 2 个模式的 context ID 计算方式为: hart_id * 2 + mode
   */
  [[nodiscard]] __always_inline auto GetContextId(uint32_t hart_id,
                                                  uint32_t mode = 1) const
      -> uint32_t {
    return hart_id * 2 + mode;
  }

  /**
   * @brief 获取使能位寄存器中指定中断源的状态
   * @param context_id context ID
   * @param source_id 中断源 ID (1-1023，0 保留)
   * @return bool 对应位的状态
   */
  [[nodiscard]] auto GetEnableBit(uint32_t context_id, uint32_t source_id) const
      -> bool;

  /**
   * @brief 设置使能位寄存器中指定中断源的状态
   * @param context_id context ID
   * @param source_id 中断源 ID (1-1023，0 保留)
   * @param value 要设置的值
   */
  auto SetEnableBit(uint32_t context_id, uint32_t source_id, bool value) const
      -> void;

  /**
   * @brief 获取中断源优先级寄存器
   * @param source_id 中断源 ID (1-1023，0 保留)
   * @return uint32_t& 寄存器引用
   */
  [[nodiscard]] auto SourcePriority(uint32_t source_id) const -> uint32_t&;

  /**
   * @brief 获取挂起位寄存器中指定中断源的状态
   * @param source_id 中断源 ID (1-1023，0 保留)
   * @return bool 对应位的状态
   */
  [[nodiscard]] auto GetPendingBit(uint32_t source_id) const -> bool;

  /**
   * @brief 设置挂起位寄存器中指定中断源的状态
   * @param source_id 中断源 ID (1-1023，0 保留)
   * @param value 要设置的值
   */
  auto SetPendingBit(uint32_t source_id, bool value) const -> void;

  /**
   * @brief 获取优先级阈值寄存器
   * @param context_id context ID
   * @return uint32_t& 寄存器引用
   */
  [[nodiscard]] auto PriorityThreshold(uint32_t context_id) const -> uint32_t&;

  /**
   * @brief 获取声明/完成寄存器
   * @param context_id context ID
   * @return uint32_t& 寄存器引用
   */
  [[nodiscard]] auto ClaimComplete(uint32_t context_id) const -> uint32_t&;
};

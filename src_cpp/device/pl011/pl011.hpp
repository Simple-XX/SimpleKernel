/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstdint>
#include <optional>

#include "mmio_accessor.hpp"

namespace pl011 {

/**
 * @brief PL011 串口驱动
 *
 * 通过 MMIO 访问 PL011 UART 寄存器，提供字符读写功能。
 * Header-only 实现，使用 MmioAccessor 进行寄存器访问。
 *
 * @see https://developer.arm.com/documentation/ddi0183/g/
 */
class Pl011 {
 public:
  /**
   * @brief 写入一个字符
   * @param c 待写入的字符
   */
  auto PutChar(uint8_t c) const -> void {
    while (mmio_.Read<uint32_t>(kRegFR) & kFRTxFIFO) {
    }
    mmio_.Write<uint32_t>(kRegDR, c);
  }

  /**
   * @brief 阻塞式读取一个字符
   * @return 读取到的字符
   */
  [[nodiscard]] auto GetChar() const -> uint8_t {
    while (mmio_.Read<uint32_t>(kRegFR) & kFRRXFE) {
    }
    return static_cast<uint8_t>(mmio_.Read<uint32_t>(kRegDR));
  }

  /**
   * @brief 非阻塞式尝试读取一个字符
   * @return 读取到的字符，如果没有数据则返回 std::nullopt
   */
  [[nodiscard]] auto TryGetChar() const -> std::optional<uint8_t> {
    if (mmio_.Read<uint32_t>(kRegFR) & kFRRXFE) {
      return std::nullopt;
    }
    return static_cast<uint8_t>(mmio_.Read<uint32_t>(kRegDR));
  }

  /**
   * @brief 检查接收缓冲区是否有数据可读（不消耗数据）
   * @return true 如果有数据可读
   */
  [[nodiscard]] auto HasData() const -> bool {
    return !(mmio_.Read<uint32_t>(kRegFR) & kFRRXFE);
  }

  /**
   * @brief 读取屏蔽后的中断状态寄存器（MIS）
   *
   * 返回经 IMSC 屏蔽后的中断状态，只包含被使能的中断源。
   *
   * @return 中断状态位掩码
   * @see ARM PL011 Technical Reference Manual
   */
  [[nodiscard]] auto GetMaskedInterruptStatus() const -> uint32_t {
    return mmio_.Read<uint32_t>(kRegMIS);
  }

  /**
   * @brief 读取原始中断状态寄存器（RIS）
   *
   * 返回所有中断源的原始状态（不经 IMSC 屏蔽）。
   *
   * @return 原始中断状态位掩码
   */
  [[nodiscard]] auto GetRawInterruptStatus() const -> uint32_t {
    return mmio_.Read<uint32_t>(kRegRIS);
  }

  /**
   * @brief 清除指定中断
   *
   * 向 ICR 寄存器写入位掩码以清除对应的中断。
   *
   * @param mask 要清除的中断位掩码
   */
  auto ClearInterrupt(uint32_t mask) const -> void {
    mmio_.Write<uint32_t>(kRegICR, mask);
  }

  /**
   * @brief 检查是否有中断挂起
   * @return true 如果有屏蔽后的中断挂起
   */
  [[nodiscard]] auto IsInterruptPending() const -> bool {
    return GetMaskedInterruptStatus() != 0;
  }

  /**
   * @brief 处理串口接收中断，对每个收到的字符调用回调
   * @param callback 字符接收回调，签名 void(uint8_t ch)
   */
  template <typename Callback>
  auto HandleInterrupt(Callback&& callback) -> void {
    while (HasData()) {
      callback(GetChar());
    }
  }

  /// @name 构造/析构函数
  /// @{
  /**
   * @brief 构造函数
   * @param dev_addr  设备 MMIO 基地址
   * @param clock     串口时钟（0 表示不设置波特率）
   * @param baud_rate 波特率（0 表示不设置波特率）
   */
  explicit Pl011(uint64_t dev_addr, uint64_t clock = 0, uint64_t baud_rate = 0)
      : mmio_(dev_addr), base_clock_(clock), baud_rate_(baud_rate) {
    mmio_.Write<uint32_t>(kRegRSRECR, 0);
    mmio_.Write<uint32_t>(kRegCR, 0);

    if (baud_rate_ != 0) {
      uint32_t divisor = (base_clock_ * 4) / baud_rate_;
      mmio_.Write<uint32_t>(kRegIBRD, divisor >> 6);
      mmio_.Write<uint32_t>(kRegFBRD, divisor & 0x3f);
    }

    mmio_.Write<uint32_t>(kRegLCRH, kLCRHWlen8);
    mmio_.Write<uint32_t>(kRegIMSC, kIMSCRxim);
    mmio_.Write<uint32_t>(kRegCR, kCREnable | kCRTxEnable | kCRRxEnable);
  }
  Pl011() = default;
  Pl011(const Pl011&) = delete;
  Pl011(Pl011&&) = default;
  auto operator=(const Pl011&) -> Pl011& = delete;
  auto operator=(Pl011&&) -> Pl011& = default;
  ~Pl011() = default;
  /// @}

 private:
  /// data register
  static constexpr uint32_t kRegDR = 0x00;
  /// receive status or error clear
  static constexpr uint32_t kRegRSRECR = 0x04;
  /// flag register
  static constexpr uint32_t kRegFR = 0x18;
  /// integer baud register
  static constexpr uint32_t kRegIBRD = 0x24;
  /// fractional baud register
  static constexpr uint32_t kRegFBRD = 0x28;
  /// line control register
  static constexpr uint32_t kRegLCRH = 0x2C;
  /// control register
  static constexpr uint32_t kRegCR = 0x30;
  /// interrupt mask set/clear
  static constexpr uint32_t kRegIMSC = 0x38;
  /// raw interrupt status register
  static constexpr uint32_t kRegRIS = 0x3C;
  /// masked interrupt status register
  static constexpr uint32_t kRegMIS = 0x40;
  /// interrupt clear register
  static constexpr uint32_t kRegICR = 0x44;

  /// flag register bits
  static constexpr uint32_t kFRTxFIFO = (1 << 5);
  static constexpr uint32_t kFRRXFE = (1 << 4);

  /// line control register bits
  static constexpr uint32_t kLCRHWlen8 = (3 << 5);

  /// control register bits
  static constexpr uint32_t kCREnable = (1 << 0);
  static constexpr uint32_t kCRTxEnable = (1 << 8);
  static constexpr uint32_t kCRRxEnable = (1 << 9);

  /// interrupt mask bits
  static constexpr uint32_t kIMSCRxim = (1 << 4);

  MmioAccessor mmio_{};
  uint64_t base_clock_{0};
  uint64_t baud_rate_{0};
};

}  // namespace pl011

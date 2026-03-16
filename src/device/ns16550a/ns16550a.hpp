/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstdint>
#include <optional>

#include "expected.hpp"
#include "mmio_accessor.hpp"

namespace ns16550a {

/**
 * @brief NS16550A 串口驱动
 *
 * 通过 MMIO 访问 NS16550A UART 寄存器，提供字符读写功能。
 * Header-only 实现，使用 MmioAccessor 进行寄存器访问。
 *
 * 使用工厂方法 Create() 构造，将验证与初始化分离。
 */
class Ns16550a {
 public:
  /**
   * @brief 工厂方法：创建并初始化 NS16550A 驱动
   * @param dev_addr 设备 MMIO 基地址
   * @return 成功返回已初始化的 Ns16550a 实例，失败返回错误
   */
  [[nodiscard]] static auto Create(uint64_t dev_addr) -> Expected<Ns16550a> {
    if (dev_addr == 0) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }

    Ns16550a uart(dev_addr);

    // UART 初始化序列
    uart.mmio_.Write<uint8_t>(kRegIER, 0x00);   // 禁用所有中断
    uart.mmio_.Write<uint8_t>(kRegLCR, 0x80);   // 启用 DLAB（设置波特率）
    uart.mmio_.Write<uint8_t>(kUartDLL, 0x03);  // 波特率低字节（38400）
    uart.mmio_.Write<uint8_t>(kUartDLM, 0x00);  // 波特率高字节
    uart.mmio_.Write<uint8_t>(kRegLCR, 0x03);   // 8 位，无校验，1 停止位
    uart.mmio_.Write<uint8_t>(kRegFCR, 0x07);   // 启用并清除 FIFO
    uart.mmio_.Write<uint8_t>(kRegIER, 0x01);   // 启用接收中断

    return uart;
  }

  /**
   * @brief 写入一个字符
   * @param c 待写入的字符
   */
  auto PutChar(uint8_t c) const -> void {
    while ((mmio_.Read<uint8_t>(kRegLSR) & (1 << 5)) == 0) {
    }
    mmio_.Write<uint8_t>(kRegTHR, c);
  }

  /**
   * @brief 阻塞式读取一个字符
   * @return 读取到的字符
   */
  [[nodiscard]] auto GetChar() const -> uint8_t {
    while ((mmio_.Read<uint8_t>(kRegLSR) & (1 << 0)) == 0) {
    }
    return mmio_.Read<uint8_t>(kRegRHR);
  }

  /**
   * @brief 非阻塞式尝试读取一个字符
   * @return 读取到的字符，如果没有数据则返回 std::nullopt
   */
  [[nodiscard]] auto TryGetChar() const -> std::optional<uint8_t> {
    if ((mmio_.Read<uint8_t>(kRegLSR) & (1 << 0)) != 0) {
      return mmio_.Read<uint8_t>(kRegRHR);
    }
    return std::nullopt;
  }

  /**
   * @brief 检查接收缓冲区是否有数据可读（不消耗数据）
   * @return true 如果有数据可读
   */
  [[nodiscard]] auto HasData() const -> bool {
    return (mmio_.Read<uint8_t>(kRegLSR) & (1 << 0)) != 0;
  }

  /**
   * @brief 读取中断标识寄存器（ISR / IIR）
   *
   * 返回值 bit[0]：0=有中断挂起，1=无中断挂起
   * 返回值 bit[3:1]：中断源标识
   *   - 0b011: 接收线状态错误
   *   - 0b010: 接收数据就绪
   *   - 0b110: 字符超时
   *   - 0b001: THR 空（发送就绪）
   *   - 0b000: Modem 状态变化
   *
   * @return 中断标识寄存器值
   */
  [[nodiscard]] auto GetInterruptId() const -> uint8_t {
    return mmio_.Read<uint8_t>(kRegISR);
  }

  /**
   * @brief 检查是否有中断挂起
   * @return true 如果有中断挂起
   */
  [[nodiscard]] auto IsInterruptPending() const -> bool {
    return (mmio_.Read<uint8_t>(kRegISR) & 0x01) == 0;
  }

  /// @name 构造/析构函数
  /// @{
  Ns16550a() = default;
  Ns16550a(const Ns16550a&) = delete;
  Ns16550a(Ns16550a&&) = default;
  auto operator=(const Ns16550a&) -> Ns16550a& = delete;
  auto operator=(Ns16550a&&) -> Ns16550a& = default;
  ~Ns16550a() = default;
  /// @}

 private:
  /// read mode: Receive holding reg
  static constexpr uint8_t kRegRHR = 0;
  /// write mode: Transmit Holding Reg
  static constexpr uint8_t kRegTHR = 0;
  /// write mode: interrupt enable reg
  static constexpr uint8_t kRegIER = 1;
  /// write mode: FIFO control Reg
  static constexpr uint8_t kRegFCR = 2;
  /// read mode: Interrupt Status Reg
  static constexpr uint8_t kRegISR = 2;
  /// write mode: Line Control Reg
  static constexpr uint8_t kRegLCR = 3;
  /// write mode: Modem Control Reg
  static constexpr uint8_t kRegMCR = 4;
  /// read mode: Line Status Reg
  static constexpr uint8_t kRegLSR = 5;
  /// read mode: Modem Status Reg
  static constexpr uint8_t kRegMSR = 6;

  /// LSB of divisor Latch when enabled
  static constexpr uint8_t kUartDLL = 0;
  /// MSB of divisor Latch when enabled
  static constexpr uint8_t kUartDLM = 1;

  MmioAccessor mmio_{};

  explicit Ns16550a(uint64_t dev_addr) : mmio_(dev_addr) {}
};

}  // namespace ns16550a

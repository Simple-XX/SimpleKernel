
/**
 * @file ns16550a.h
 * @brief ns16550a 头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-05-24
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-05-24<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_DRIVER_NS16550A_INCLUDE_NS16550A_H_
#define SIMPLEKERNEL_SRC_KERNEL_DRIVER_NS16550A_INCLUDE_NS16550A_H_

#include <cstdint>

class Ns16550a {
 public:
  /**
   * 构造函数
   * @param dev_addr 设备地址
   */
  explicit Ns16550a(uintptr_t dev_addr);

  /// @name 默认构造/析构函数
  /// @{
  Ns16550a() = delete;
  Ns16550a(const Ns16550a& na16550a) = delete;
  Ns16550a(Ns16550a&& na16550a) = delete;
  auto operator=(const Ns16550a& na16550a) -> Ns16550a& = delete;
  auto operator=(Ns16550a&& na16550a) -> Ns16550a& = delete;
  ~Ns16550a() = default;
  /// @}

  void PutChar(uint8_t c);

 private:
  /// read mode: Receive holding reg
  static constexpr const uint8_t kRegRHR = 0;
  /// write mode: Transmit Holding Reg
  static constexpr const uint8_t kRegTHR = 0;
  /// write mode: interrupt enable reg
  static constexpr const uint8_t kRegIER = 1;
  /// write mode: FIFO control Reg
  static constexpr const uint8_t kRegFCR = 2;
  /// read mode: Interrupt Status Reg
  static constexpr const uint8_t kRegISR = 2;
  /// write mode:Line Control Reg
  static constexpr const uint8_t kRegLCR = 3;
  /// write mode:Modem Control Reg
  static constexpr const uint8_t kRegMCR = 4;
  /// read mode: Line Status Reg
  static constexpr const uint8_t kRegLSR = 5;
  /// read mode: Modem Status Reg
  static constexpr const uint8_t kRegMSR = 6;

  /// LSB of divisor Latch when enabled
  static constexpr const uint8_t kUartDLL = 0;
  /// MSB of divisor Latch when enabled
  static constexpr const uint8_t kUartDLM = 1;

  uintptr_t base_addr_;

  inline volatile uint8_t* Reg(uint8_t reg);

  inline uint8_t Read(uint8_t reg);

  inline void Write(uint8_t reg, uint8_t c);
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_DRIVER_NS16550A_INCLUDE_NS16550A_H_ */

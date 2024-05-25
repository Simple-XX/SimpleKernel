
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

class na16550a_t {
 public:
  /**
   * 构造函数
   * @param _dev_addr 设备地址
   */
  explicit na16550a_t(uintptr_t _dev_addr);

  /// @name 默认构造/析构函数
  /// @{
  na16550a_t() = delete;
  na16550a_t(const na16550a_t& _na16550a) = delete;
  na16550a_t(na16550a_t&& _na16550a) = delete;
  auto operator=(const na16550a_t& _na16550a) -> na16550a_t& = delete;
  auto operator=(na16550a_t&& _na16550a) -> na16550a_t& = delete;
  ~na16550a_t() = default;
  /// @}

  void putc(uint8_t _c);

 private:
  /// read mode: Receive holding reg
  static constexpr const uint8_t REG_RHR = 0;
  /// write mode: Transmit Holding Reg
  static constexpr const uint8_t REG_THR = 0;
  /// write mode: interrupt enable reg
  static constexpr const uint8_t REG_IER = 1;
  /// write mode: FIFO control Reg
  static constexpr const uint8_t REG_FCR = 2;
  /// read mode: Interrupt Status Reg
  static constexpr const uint8_t REG_ISR = 2;
  /// write mode:Line Control Reg
  static constexpr const uint8_t REG_LCR = 3;
  /// write mode:Modem Control Reg
  static constexpr const uint8_t REG_MCR = 4;
  /// read mode: Line Status Reg
  static constexpr const uint8_t REG_LSR = 5;
  /// read mode: Modem Status Reg
  static constexpr const uint8_t REG_MSR = 6;

  /// LSB of divisor Latch when enabled
  static constexpr const uint8_t UART_DLL = 0;
  /// MSB of divisor Latch when enabled
  static constexpr const uint8_t UART_DLM = 1;

  uintptr_t base_addr;

  inline volatile uint8_t* Reg(uint8_t _reg);

  inline uint8_t read(uint8_t _reg);

  inline void write(uint8_t _reg, uint8_t _c);
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_DRIVER_NS16550A_INCLUDE_NS16550A_H_ */

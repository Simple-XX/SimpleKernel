
/**
 * @file cpu.hpp
 * @brief x86_64 cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-03-05
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-03-05<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP

#include "cstdint"

class CPU {
 public:
  /**
   * @brief  读一个字节
   * @param  _port           要读的端口
   * @return uint8_t         读取到的数据
   */
  static inline uint8_t inb(const uint32_t _port) {
    uint8_t data;
    __asm__ __volatile__("inb %1, %0" : "=a"(data) : "dN"(_port));
    return data;
  }

  /**
   * @brief  读一个字
   * @param  _port           要读的端口
   * @return uint16_t        读取到的数据
   */
  static inline uint16_t inw(const uint32_t _port) {
    uint16_t data;
    __asm__ __volatile__("inw %1, %0" : "=a"(data) : "dN"(_port));
    return data;
  }

  /**
   * @brief  读一个双字
   * @param  _port           要读的端口
   * @return uint32_t        读取到的数据
   */
  static inline uint32_t inl(const uint32_t _port) {
    uint32_t data;
    __asm__ __volatile__("inl %1, %0" : "=a"(data) : "dN"(_port));
    return data;
  }

  /**
   * @brief  写一个字节
   * @param  _port           要写的端口
   * @param  _data           要写的数据
   */
  static inline void outb(const uint32_t _port, const uint8_t _data) {
    __asm__ __volatile__("outb %1, %0" : : "dN"(_port), "a"(_data));
  }

  /**
   * @brief  写一个字
   * @param  _port           要写的端口
   * @param  _data           要写的数据
   */
  static inline void outw(const uint32_t _port, const uint16_t _data) {
    __asm__ __volatile__("outw %1, %0" : : "dN"(_port), "a"(_data));
  }

  /**
   * @brief  写一个双字
   * @param  _port           要写的端口
   * @param  _data           要写的数据
   */
  static inline void outl(const uint32_t _port, const uint32_t _data) {
    __asm__ __volatile__("outl %1, %0" : : "dN"(_port), "a"(_data));
  }

  /// @name 端口
  static constexpr const uint32_t COM1 = 0x3F8;
  class Serial {
   public:
    explicit Serial(uint32_t _port) : port(_port) {
      // Disable all interrupts
      outb(port + 1, 0x00);
      // Enable DLAB (set baud rate divisor)
      outb(port + 3, 0x80);
      // Set divisor to 3 (lo byte) 38400 baud
      outb(port + 0, 0x03);
      // (hi byte)
      outb(port + 1, 0x00);
      // 8 bits, no parity, one stop bit
      outb(port + 3, 0x03);
      // Enable FIFO, clear them, with 14-byte threshold
      outb(port + 2, 0xC7);
      // IRQs enabled, RTS/DSR set
      outb(port + 4, 0x0B);
      // Set in loopback mode, test the serial chip
      outb(port + 4, 0x1E);
      // Test serial chip (send byte 0xAE and check if serial returns same byte)
      outb(port + 0, 0xAE);
      // Check if serial is faulty (i.e: not same byte as sent)
      if (inb(port + 0) != 0xAE) {
        /// @todo 失败处理
      }

      // If serial is not faulty set it in normal operation mode (not-loopback
      // with IRQs enabled and OUT#1 and OUT#2 bits enabled)
      outb(port + 4, 0x0F);
    }

    ~Serial() = default;

    /// @name 不使用的构造函数
    /// @{
    Serial() = delete;
    Serial(const Serial &) = delete;
    Serial(Serial &&) = delete;
    auto operator=(const Serial &) -> Serial & = delete;
    auto operator=(Serial &&) -> Serial & = delete;
    /// @}

    /**
     * @brief  读一个字节
     * @return uint8_t         读取到的数据
     */
    [[nodiscard]] auto read() const -> uint8_t {
      while (!serial_received()) {
        ;
      }
      return inb(port);
    }

    /**
     * @brief  写一个字节
     * @param  _c              要写的数据
     */
    void write(uint8_t _c) const {
      while (!is_transmit_empty()) {
        ;
      }
      outb(port, _c);
    }

   private:
    uint32_t port;

    /**
     * @brief 串口是否接收到数据
     * @return true
     * @return false
     */
    [[nodiscard]] auto serial_received() const -> bool {
      return inb(port + 5) & 1;
    }

    /**
     * @brief 串口是否可以发送数据
     * @return true
     * @return false
     */
    [[nodiscard]] auto is_transmit_empty() const -> bool {
      return inb(port + 5) & 0x20;
    }
  };
};

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char _character) {
  auto serial = CPU::Serial(CPU::COM1);
  serial.write(_character);
}

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP

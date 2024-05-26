
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP_

#include <cstdint>

class Cpu {
 public:
  /**
   * @brief  读一个字节
   * @param  port           要读的端口
   * @return uint8_t         读取到的数据
   */
  static inline uint8_t inb(const uint32_t port) {
    uint8_t data;
    __asm__ __volatile__("inb %1, %0" : "=a"(data) : "dN"(port));
    return data;
  }

  /**
   * @brief  读一个字
   * @param  port           要读的端口
   * @return uint16_t        读取到的数据
   */
  static inline uint16_t inw(const uint32_t port) {
    uint16_t data;
    __asm__ __volatile__("inw %1, %0" : "=a"(data) : "dN"(port));
    return data;
  }

  /**
   * @brief  读一个双字
   * @param  port           要读的端口
   * @return uint32_t        读取到的数据
   */
  static inline uint32_t inl(const uint32_t port) {
    uint32_t data;
    __asm__ __volatile__("inl %1, %0" : "=a"(data) : "dN"(port));
    return data;
  }

  /**
   * @brief  写一个字节
   * @param  port           要写的端口
   * @param  data           要写的数据
   */
  static inline void outb(const uint32_t port, const uint8_t data) {
    __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(data));
  }

  /**
   * @brief  写一个字
   * @param  port           要写的端口
   * @param  data           要写的数据
   */
  static inline void outw(const uint32_t port, const uint16_t data) {
    __asm__ __volatile__("outw %1, %0" : : "dN"(port), "a"(data));
  }

  /**
   * @brief  写一个双字
   * @param  port           要写的端口
   * @param  data           要写的数据
   */
  static inline void outl(const uint32_t port, const uint32_t data) {
    __asm__ __volatile__("outl %1, %0" : : "dN"(port), "a"(data));
  }

  /// @name 端口
  static constexpr const uint32_t kCom1 = 0x3F8;
  class Serial {
   public:
    explicit Serial(uint32_t port) : port_(port) {
      // Disable all interrupts
      outb(port_ + 1, 0x00);
      // Enable DLAB (set baud rate divisor)
      outb(port_ + 3, 0x80);
      // Set divisor to 3 (lo byte) 38400 baud
      outb(port_ + 0, 0x03);
      // (hi byte)
      outb(port_ + 1, 0x00);
      // 8 bits, no parity, one stop bit
      outb(port_ + 3, 0x03);
      // Enable FIFO, clear them, with 14-byte threshold
      outb(port_ + 2, 0xC7);
      // IRQs enabled, RTS/DSR set
      outb(port_ + 4, 0x0B);
      // Set in loopback mode, test the serial chip
      outb(port_ + 4, 0x1E);
      // Test serial chip (send byte 0xAE and check if serial returns same byte)
      outb(port_ + 0, 0xAE);
      // Check if serial is faulty (i.e: not same byte as sent)
      if (inb(port_ + 0) != 0xAE) {
        /// @todo 失败处理
      }

      // If serial is not faulty set it in normal operation mode (not-loopback
      // with IRQs enabled and OUT#1 and OUT#2 bits enabled)
      outb(port_ + 4, 0x0F);
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
      return inb(port_);
    }

    /**
     * @brief  写一个字节
     * @param  c              要写的数据
     */
    void write(uint8_t c) const {
      while (!is_transmit_empty()) {
        ;
      }
      outb(port_, c);
    }

   private:
    uint32_t port_;

    /**
     * @brief 串口是否接收到数据
     * @return true
     * @return false
     */
    [[nodiscard]] auto serial_received() const -> bool {
      return inb(port_ + 5) & 1;
    }

    /**
     * @brief 串口是否可以发送数据
     * @return true
     * @return false
     */
    [[nodiscard]] auto is_transmit_empty() const -> bool {
      return inb(port_ + 5) & 0x20;
    }
  };
};

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_CPU_HPP_

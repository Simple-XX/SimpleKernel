
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "cstdio"
#include "iostream"
#include "kernel_log.hpp"

namespace cpu {
/**
 * @brief  读一个字节
 * @param  port           要读的端口
 * @return uint8_t         读取到的数据
 */
static __always_inline uint8_t InByte(const uint32_t port) {
  uint8_t data;
  __asm__ volatile("inb %1, %0" : "=a"(data) : "dN"(port));
  return data;
}

/**
 * @brief  读一个字
 * @param  port           要读的端口
 * @return uint16_t        读取到的数据
 */
static __always_inline uint16_t InWord(const uint32_t port) {
  uint16_t data;
  __asm__ volatile("inw %1, %0" : "=a"(data) : "dN"(port));
  return data;
}

/**
 * @brief  读一个双字
 * @param  port           要读的端口
 * @return uint32_t        读取到的数据
 */
static __always_inline uint32_t InLong(const uint32_t port) {
  uint32_t data;
  __asm__ volatile("inl %1, %0" : "=a"(data) : "dN"(port));
  return data;
}

/**
 * @brief  写一个字节
 * @param  port           要写的端口
 * @param  data           要写的数据
 */
static __always_inline void OutByte(const uint32_t port, const uint8_t data) {
  __asm__ volatile("outb %1, %0" : : "dN"(port), "a"(data));
}

/**
 * @brief  写一个字
 * @param  port           要写的端口
 * @param  data           要写的数据
 */
static __always_inline void OutWord(const uint32_t port, const uint16_t data) {
  __asm__ volatile("outw %1, %0" : : "dN"(port), "a"(data));
}

/**
 * @brief  写一个双字
 * @param  port           要写的端口
 * @param  data           要写的数据
 */
static __always_inline void OutLong(const uint32_t port, const uint32_t data) {
  __asm__ volatile("outl %1, %0" : : "dN"(port), "a"(data));
}

/// @name 端口
static constexpr const uint32_t kCom1 = 0x3F8;
/**
 * 串口定义
 */
class Serial {
 public:
  explicit Serial(uint32_t port) : port_(port) {
    // Disable all interrupts
    OutByte(port_ + 1, 0x00);
    // Enable DLAB (set baud rate divisor)
    OutByte(port_ + 3, 0x80);
    // Set divisor to 3 (lo byte) 38400 baud
    OutByte(port_ + 0, 0x03);
    // (hi byte)
    OutByte(port_ + 1, 0x00);
    // 8 bits, no parity, one stop bit
    OutByte(port_ + 3, 0x03);
    // Enable FIFO, clear them, with 14-byte threshold
    OutByte(port_ + 2, 0xC7);
    // IRQs enabled, RTS/DSR set
    OutByte(port_ + 4, 0x0B);
    // Set in loopback mode, test the serial chip
    OutByte(port_ + 4, 0x1E);
    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    OutByte(port_ + 0, 0xAE);
    // Check if serial is faulty (i.e: not same byte as sent)
    if (InByte(port_ + 0) != 0xAE) {
      asm("hlt");
    }

    // If serial is not faulty set it in normal operation mode (not-loopback
    // with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    OutByte(port_ + 4, 0x0F);
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
  [[nodiscard]] auto Read() const -> uint8_t {
    while (!SerialReceived()) {
      ;
    }
    return InByte(port_);
  }

  /**
   * @brief  写一个字节
   * @param  c              要写的数据
   */
  void Write(uint8_t c) const {
    while (!IsTransmitEmpty()) {
      ;
    }
    OutByte(port_, c);
  }

 private:
  uint32_t port_;

  /**
   * @brief 串口是否接收到数据
   * @return true
   * @return false
   */
  [[nodiscard]] auto SerialReceived() const -> bool {
    return InByte(port_ + 5) & 1;
  }

  /**
   * @brief 串口是否可以发送数据
   * @return true
   * @return false
   */
  [[nodiscard]] auto IsTransmitEmpty() const -> bool {
    return InByte(port_ + 5) & 0x20;
  }
};

/// 第一部分：寄存器定义
namespace reginfo {

struct RegInfoBase {
  using DataType = uint64_t;
  static constexpr uint64_t kBitOffset = 0;
  static constexpr uint64_t kBitWidth = 64;
  static constexpr uint64_t kBitMask = ~0;
  static constexpr uint64_t kAllSetMask = ~0;
};

/// 通用寄存器
struct RbpInfo : public RegInfoBase {};

};  // namespace reginfo

/// 第二部分：读/写模版实现
/**
 * 只读接口
 * @tparam 寄存器类型
 */
template <class Reg>
class ReadOnlyRegBase {
 public:
  /// @name 构造/析构函数
  /// @{
  ReadOnlyRegBase() = default;
  ReadOnlyRegBase(const ReadOnlyRegBase &) = delete;
  ReadOnlyRegBase(ReadOnlyRegBase &&) = delete;
  auto operator=(const ReadOnlyRegBase &) -> ReadOnlyRegBase & = delete;
  auto operator=(ReadOnlyRegBase &&) -> ReadOnlyRegBase & = delete;
  ~ReadOnlyRegBase() = default;
  /// @}

  /**
   * 读寄存器
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t Read() {
    uint64_t value = -1;
    if constexpr (std::is_same<Reg, reginfo::RbpInfo>::value) {
      __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else {
      Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * () 重载
   */
  static __always_inline uint64_t operator()() { return Read(); }
};

/**
 * 只写接口
 * @tparam 寄存器类型
 */
template <class Reg>
class WriteOnlyRegBase {
 public:
  /// @name 构造/析构函数
  /// @{
  WriteOnlyRegBase() = default;
  WriteOnlyRegBase(const WriteOnlyRegBase &) = delete;
  WriteOnlyRegBase(WriteOnlyRegBase &&) = delete;
  auto operator=(const WriteOnlyRegBase &) -> WriteOnlyRegBase & = delete;
  auto operator=(WriteOnlyRegBase &&) -> WriteOnlyRegBase & = delete;
  ~WriteOnlyRegBase() = default;
  /// @}

  /**
   * 写寄存器
   * @param value 要写的值
   */
  static __always_inline void Write(uint64_t value) {
    if constexpr (std::is_same<Reg, reginfo::RbpInfo>::value) {
      __asm__ volatile("mv fp, %0" : : "r"(value) :);
    } else {
      Err("No Type\n");
      throw;
    }
  }
};

/**
 * 读写接口
 * @tparam 寄存器类型
 */
template <class Reg>
class ReadWriteRegBase : public ReadOnlyRegBase<Reg>,
                         public WriteOnlyRegBase<Reg> {
 public:
  /// @name 构造/析构函数
  /// @{
  ReadWriteRegBase() = default;
  ReadWriteRegBase(const ReadWriteRegBase &) = delete;
  ReadWriteRegBase(ReadWriteRegBase &&) = delete;
  auto operator=(const ReadWriteRegBase &) -> ReadWriteRegBase & = delete;
  auto operator=(ReadWriteRegBase &&) -> ReadWriteRegBase & = delete;
  ~ReadWriteRegBase() = default;
  /// @}
};

/// 第三部分：寄存器实例
class Rbp : public ReadWriteRegBase<reginfo::RbpInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Rbp() = default;
  Rbp(const Rbp &) = delete;
  Rbp(Rbp &&) = delete;
  auto operator=(const Rbp &) -> Rbp & = delete;
  auto operator=(Rbp &&) -> Rbp & = delete;
  virtual ~Rbp() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Rbp &rbp) {
    printf("val: 0x%p", rbp.Read());
    return os;
  }
};

/// 第四部分：访问接口
/**
 * @brief 通用寄存器
 */
class AllXreg {
 public:
  Rbp rbp;

  /// @name 构造/析构函数
  /// @{
  AllXreg() = default;
  AllXreg(const AllXreg &) = delete;
  AllXreg(AllXreg &&) = delete;
  auto operator=(const AllXreg &) -> AllXreg & = delete;
  auto operator=(AllXreg &&) -> AllXreg & = delete;
  virtual ~AllXreg() = default;
  /// @}
};

static AllXreg kAllXreg;

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_HPP_

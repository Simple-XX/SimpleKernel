
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
#include <type_traits>
#include <typeinfo>

#include "cstdio"
#include "iostream"

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

/**
 * 读 rbp 寄存器
 * @return rbp 寄存器的值
 */
static __always_inline uint64_t ReadRbp() {
  uint64_t rbp = -1;
  __asm__ volatile("mov %%rbp, %0" : "=r"(rbp));
  return rbp;
}

namespace cr {

struct CrRegInfoBase {
  using DataType = uint64_t;
  static constexpr uint64_t kBitOffset = 0;
  static constexpr uint64_t kBitWidth = 64;
  static constexpr uint64_t kBitMask = ~0;
  static constexpr uint64_t kAllSetMask = ~0;
};

/**
 * @brief cr0 寄存器定义
 * @see
 */
struct Cr0Info : public CrRegInfoBase {
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x2;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
};

/**
 * @brief cr2 寄存器定义
 * @see
 */
struct Cr2Info : public CrRegInfoBase {
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x2;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
};

/**
 * @brief cr3 寄存器定义
 * @see
 */
struct Cr3Info : public CrRegInfoBase {
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x2;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
};

/**
 * @brief cr4 寄存器定义
 * @see
 */
struct Cr4Info : public CrRegInfoBase {
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x2;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
};

/**
 * @brief cr8 寄存器定义
 * @see
 */
struct Cr8Info : public CrRegInfoBase {
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x2;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
};

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
   * 读 cr 寄存器
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t Read() {
    uint64_t value = -1;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrr %0, sstatus" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrr %0, stvec" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrr %0, sip" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrr %0, sie" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrr %0, time" : "=r"(value) : :);
    } else {
      printf("error\n");
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
   * 写 cr 寄存器
   * @param value 要写的值
   */
  static __always_inline void Write(uint64_t value) {
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrw sstatus, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrw stvec, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrw sip, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrw sie, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrw sscratch, %0" : : "r"(value) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 写 csr 寄存器，不通过寄存器中转
   * @param value 要写的值
   */
  static __always_inline void WriteImm(const uint8_t value) {
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrwi sstatus, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrwi stvec, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrwi sip, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrwi sie, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrwi sscratch, %0" : : "i"(value) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后写寄存器
   * @param value 要写的值
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadWrite(uint64_t value) {
    uint64_t old_value;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrw %0, sstatus, %1"
                       : "=r"(old_value)
                       : "r"(value)
                       :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrw %0, stvec, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrw %0, sip, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrw %0, sie, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrw %0, sscratch, %1"
                       : "=r"(old_value)
                       : "r"(value)
                       :);
    } else {
      printf("error\n");
    }
    return old_value;
  }

  /**
   * 先读后写寄存器，不通过寄存器中转
   * @param value 要写的值
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadWriteImm(const uint8_t value) {
    uint64_t old_value;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrwi %0, sstatus, %1"
                       : "=r"(old_value)
                       : "i"(value)
                       :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrwi %0, stvec, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrwi %0, sip, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrwi %0, sie, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrwi %0, sscratch, %1"
                       : "=r"(old_value)
                       : "i"(value)
                       :);
    } else {
      printf("error\n");
    }
    return old_value;
  }

  /**
   * 通过掩码设置寄存器
   * @param mask 掩码
   */
  static __always_inline void SetBits(uint64_t mask) {
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrs zero, sstatus, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrs zero, stvec, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrs zero, sip, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrs zero, sie, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrs zero, sscratch, %0" : : "r"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后通过掩码设置寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadSetBits(uint64_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrs %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrs %0, stvec, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrs %0, sip, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrs %0, sie, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrs %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 清零寄存器
   * @param mask 掩码
   */
  static __always_inline void ClearBits(uint64_t mask) {
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrc zero, sstatus, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrc zero, stvec, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrc zero, sip, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrc zero, sie, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrc zero, sscratch, %0" : : "r"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后清零寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadClearBits(uint64_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrc %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrc %0, stvec, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrc %0, sip, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrc %0, sie, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrc %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 通过掩码设置寄存器，不通过寄存器中转
   * @param mask 掩码
   */
  static __always_inline void SetBitsImm(const uint8_t mask) {
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrsi zero, sstatus, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrsi zero, stvec, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrsi zero, sip, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrsi zero, sie, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrsi zero, sscratch, %0" : : "i"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后通过掩码设置寄存器，不通过寄存器中转
   * @param mask 掩码
   */
  static __always_inline uint64_t ReadSetBitsImm(const uint8_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrsi %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrsi %0, stvec, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrsi %0, sip, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrsi %0, sie, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrsi %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 清零寄存器，不通过寄存器中转
   * @param mask 掩码
   */
  static __always_inline void ClearBitsImm(const uint8_t mask) {
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrci zero, sstatus, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrci zero, stvec, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrci zero, sip, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrci zero, sie, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrci zero, sscratch, %0" : : "i"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后清零寄存器，不通过寄存器中转
   * @param mask 掩码
   */
  static __always_inline uint64_t ReadClearBitsImm(const uint8_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, Cr0Info>::value) {
      __asm__ volatile("csrrci %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr2Info>::value) {
      __asm__ volatile("csrrci %0, stvec, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr3Info>::value) {
      __asm__ volatile("csrrci %0, sip, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr4Info>::value) {
      __asm__ volatile("csrrci %0, sie, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, Cr8Info>::value) {
      __asm__ volatile("csrrci %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 向寄存器写常数
   * @tparam value 常数的值
   */
  template <uint64_t value>
  static void WriteConst() {
    // if constexpr ((value & kCsrImmOpMask) == value) {
    WriteImm(value);
    // } else {
    // Write(value);
    // }
  }

  /**
   * 通过掩码写寄存器
   * @tparam mask 掩码
   */
  template <uint64_t mask>
  static void SetConst() {
    // if constexpr ((mask & kCsrImmOpMask) == mask) {
    SetBitsImm(mask);
    // } else {
    // SetBits(mask);
    // }
  }

  /**
   * 通过掩码清零寄存器
   * @tparam mask 掩码
   */
  template <uint64_t mask>
  static void ClearConst() {
    // if constexpr ((mask & kCsrImmOpMask) == mask) {
    ClearBitsImm(mask);
    // } else {
    // ClearBits(mask);
    // }
  }

  /**
   * |= 重载
   */
  __always_inline void operator|=(uint64_t mask) { SetBits(mask); }
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

  /**
   * 先读后写常数到寄存器
   * @tparam value 要写的值
   * @return uint64_t 寄存器的值
   */
  template <uint64_t value>
  static uint64_t ReadWriteConst() {
    // if constexpr ((value & kCsrImmOpMask) == value) {
    return ReadWriteRegBase<Reg>::ReadWriteImm(value);
    // } else {
    // return ReadWrite(value);
    // }
  }

  /**
   * 先读后写寄存器
   * @tparam value 要写的值
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadWrite(uint64_t value) {
    return ReadWrite(value);
  }

  /**
   * 通过常数掩码先读后写寄存器
   * @tparam mask 掩码
   * @return uint64_t 寄存器的值
   */
  template <uint64_t mask>
  static uint64_t ReadSetBitsConst() {
    // if constexpr ((mask & kCsrImmOpMask) == mask) {
    return WriteOnlyRegBase<Reg>::ReadSetBitsImm(mask);
    // } else {
    // return ReadSetBits(mask);
    // }
  }

  /**
   * 通过掩码先读后写寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadSetBits(uint64_t mask) {
    return ReadSetBits(mask);
  }

  /**
   * 通过常数掩码先读后清零寄存器
   * @tparam mask 掩码
   * @return uint64_t 寄存器的值
   */
  template <uint64_t mask>
  static uint64_t ReadClearBitsConst() {
    // if constexpr ((mask & kCsrImmOpMask) == mask) {
    return WriteOnlyRegBase<Reg>::ReadClearBitsImm(mask);
    // } else {
    // return ReadClearBits(mask);
    // }
  }

  /**
   * 通过掩码先读后清零寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadClearBits(uint64_t mask) {
    return ReadClearBits(mask);
  }
};

/**
 * 只读位域接口
 * @tparam Reg 寄存器类型
 * @tparam Info 寄存器数据信息
 */
template <class Reg, class Info>
class ReadOnlyField {
 public:
  /// @name 构造/析构函数
  /// @{
  ReadOnlyField() = default;
  ReadOnlyField(const ReadOnlyField &) = delete;
  ReadOnlyField(ReadOnlyField &&) = delete;
  auto operator=(const ReadOnlyField &) -> ReadOnlyField & = delete;
  auto operator=(ReadOnlyField &&) -> ReadOnlyField & = delete;
  ~ReadOnlyField() = default;
  /// @}

  /**
   * 获取对应 Reg 的由 Info 规定的指定位的值
   * @return Info::DataType 指定位值的信息
   */
  static __always_inline Info::DataType Get() {
    return (typename Info::DataType)((Reg::Read() & Info::kBitMask) >>
                                     Info::kBitOffset);
  }

  /**
   * 从指定的值获取对应 Reg 的由 Info 规定的指定位的值
   * @param value 指定的值
   * @return Info::DataType 指定位值的信息
   */
  static __always_inline Info::DataType Get(uint64_t value) {
    return (typename Info::DataType)((value & Info::kBitMask) >>
                                     Info::kBitOffset);
  }
};

/**
 * 只写位域接口
 * @tparam Reg 寄存器类型
 * @tparam Info 寄存器数据信息
 */
template <class Reg, class Info>
class WriteOnlyField {
 public:
  /// @name 构造/析构函数
  /// @{
  WriteOnlyField() = default;
  WriteOnlyField(const WriteOnlyField &) = delete;
  WriteOnlyField(WriteOnlyField &&) = delete;
  auto operator=(const WriteOnlyField &) -> WriteOnlyField & = delete;
  auto operator=(WriteOnlyField &&) -> WriteOnlyField & = delete;
  ~WriteOnlyField() = default;
  /// @}

  /**
   * 置位对应 Reg 的由 Info 规定的指定位
   */
  static __always_inline void Set() {
    // if constexpr ((Info::kBitMask & kCsrImmOpMask) == Info::kBitMask) {
    Reg::SetBitsImm(Info::kBitMask);
    // } else {
    // Reg::SetBits(Info::kBitMask);
    // }
  }

  /**
   * 清零对应 Reg 的由 Info 规定的指定位
   */
  static __always_inline void Clear() {
    // if constexpr ((Info::kBitMask & kCsrImmOpMask) == Info::kBitMask) {
    Reg::ClearBitsImm(Info::kBitMask);
    // } else {
    // Reg::ClearBits(Info::kBitMask);
    // }
  }
};

/**
 * 读写位域接口
 * @tparam Reg 寄存器类型
 * @tparam Info 寄存器数据信息
 */
template <class Reg, class Info>
class ReadWriteField : public ReadOnlyField<Reg, Info>,
                       public WriteOnlyField<Reg, Info> {
 public:
  /// @name 构造/析构函数
  /// @{
  ReadWriteField() = default;
  ReadWriteField(const ReadWriteField &) = delete;
  ReadWriteField(ReadWriteField &&) = delete;
  auto operator=(const ReadWriteField &) -> ReadWriteField & = delete;
  auto operator=(ReadWriteField &&) -> ReadWriteField & = delete;
  ~ReadWriteField() = default;
  /// @}

  /**
   * 将寄存器的原值替换为指定值
   * @param value 新值
   */
  static __always_inline void Write(Info::DataType value) {
    auto org_value = Reg::Read();
    auto new_value = (org_value & ~Info::kBitMask) |
                     ((value << Info::kBitOffset) & Info::kBitMask);
    Reg::Write(new_value);
  }

  /**
   * 先读出旧值，后将寄存器的值替换为指定值
   * @param value 新值
   * @return Info::DataType 由寄存器规定的数据类型
   */
  static __always_inline Info::DataType ReadWrite(Info::DataType value) {
    auto org_value = Reg::Read();
    auto new_value = (org_value & ~Info::kBitMask) |
                     ((value << Info::kBitOffset) & Info::kBitMask);
    Reg::Write(new_value);
    return (Info::DataType)((org_value & Info::kBitMask) >> Info::kBitOffset);
  }
};

};  // namespace cr

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_HPP_

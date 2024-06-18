
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

/**
 * x86_64 cpu 相关定义
 * @note 寄存器读写设计见 arch/README.md
 * @see sdm.pdf
 * Intel® 64 and IA-32 Architectures Software Developer’s Manual
 * Volume 3 (3A, 3B, 3C, & 3D): System Programming Guide
 * Order Number: 325384-083US
 * https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
 */
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

// 第一部分：寄存器定义
namespace reginfo {

struct RegInfoBase {
  /// 寄存器数据类型
  using DataType = uint64_t;
  /// 起始位
  static constexpr uint64_t kBitOffset = 0;
  /// 位宽
  static constexpr uint64_t kBitWidth = 64;
  /// 掩码，(val & kBitMask) == 对应当前位的值
  static constexpr uint64_t kBitMask =
      (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
  /// 对应位置位掩码
  static constexpr uint64_t kAllSetMask =
      (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
};

/// 通用寄存器
struct RbpInfo : public RegInfoBase {};

/**
 * @brief efer 寄存器
 * @see sdm.pdf#2.2.1
 */
struct EferInfo : public RegInfoBase {
  struct Sce {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Lme {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 8;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  /// @note Read Only
  struct Lma {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 10;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Nxe {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 11;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

/**
 * @brief rflags 寄存器
 * @see sdm.pdf#2.3
 */
struct RflagsInfo : public RegInfoBase {
  struct If {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 9;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

/**
 * @brief gdtr 寄存器
 * @see sdm.pdf#2.4.1
 */
struct GdtrInfo : public RegInfoBase {};

/**
 * @brief ldtr 寄存器
 * @see sdm.pdf#2.4.2
 */
struct LdtrInfo : public RegInfoBase {};

/**
 * @brief idtr 寄存器
 * @see sdm.pdf#2.4.3
 */
struct IdtrInfo : public RegInfoBase {};

/**
 * @brief tr 寄存器
 * @see sdm.pdf#2.4.4
 */
struct TrInfo : public RegInfoBase {};

/**
 * @brief cr 寄存器
 * @see sdm.pdf#2.5
 */
namespace cr {

struct Cr0Info : public RegInfoBase {
  struct Pe {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Pg {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 31;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

struct Cr2Info : public RegInfoBase {};

struct Cr3Info : public RegInfoBase {
  struct Pwt {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 3;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Pcd {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 4;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct PageDirectoryBase {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 12;
    static constexpr uint64_t kBitWidth = 52;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

struct Cr4Info : public RegInfoBase {
  struct Pae {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 5;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

struct Cr8Info : public RegInfoBase {};

};  // namespace cr

/**
 * @brief cpuid 寄存器
 * @see sdm.pdf#2.5.1
 */
struct CpuidInfo : public RegInfoBase {};

/**
 * @brief xcr0 寄存器
 * @see sdm.pdf#2.6
 */
struct Xcr0Info : public RegInfoBase {};

};  // namespace reginfo

// 第二部分：读/写模版实现
namespace {
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
    } else if constexpr (std::is_same<Reg, reginfo::EferInfo>::value) {
      // __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::RflagsInfo>::value) {
      __asm__ volatile("pushfq; popq %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::GdtrInfo>::value) {
      // __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::LdtrInfo>::value) {
      // __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::IdtrInfo>::value) {
      // __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::TrInfo>::value) {
      // __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("mov %%cr0, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("mov %%cr2, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("mov %%cr3, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("mov %%cr4, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("mov %%cr8, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::CpuidInfo>::value) {
      __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, reginfo::Xcr0Info>::value) {
      // __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
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
      __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::EferInfo>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::RflagsInfo>::value) {
      __asm__ volatile("pushq %0; popfq" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::GdtrInfo>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::LdtrInfo>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::IdtrInfo>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::TrInfo>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("mov %0, %%cr0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("mov %0, %%cr2" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("mov %0, %%cr3" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("mov %0, %%cr4" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("mov %0, %%cr8" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, reginfo::Xcr0Info>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else {
      Err("No Type\n");
      throw;
    }
  }

  /**
   * 通过偏移设置寄存器
   * @param offset 位偏移
   */
  static __always_inline void SetBits(uint64_t offset) {
    if constexpr (std::is_same<Reg, reginfo::RbpInfo>::value) {
      __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::EferInfo>::value) {
      // __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::RflagsInfo>::value) {
      // __asm__ volatile("pushq %0; popfq" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::GdtrInfo>::value) {
      // __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::LdtrInfo>::value) {
      // __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::IdtrInfo>::value) {
      // __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::TrInfo>::value) {
      // __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("bts %%cr0, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("bts %%cr2, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("bts %%cr3, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("bts %%cr4, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("bts %%cr8, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::Xcr0Info>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(offset) :);
    } else {
      Err("No Type\n");
      throw;
    }
  }

  /**
   * 清零寄存器
   * @param offset 位偏移
   */
  static __always_inline void ClearBits(uint64_t offset) {
    if constexpr (std::is_same<Reg, reginfo::RbpInfo>::value) {
      __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::EferInfo>::value) {
      // __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::RflagsInfo>::value) {
      // __asm__ volatile("pushq %0; popfq" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::GdtrInfo>::value) {
      // __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::LdtrInfo>::value) {
      // __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::IdtrInfo>::value) {
      // __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::TrInfo>::value) {
      // __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("btr %%cr0, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("btr %%cr2, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("btr %%cr3, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("btr %%cr4, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("btr %%cr8, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<Reg, reginfo::Xcr0Info>::value) {
      // __asm__ volatile("mov %0, %%rbp" : : "r"(offset) :);
    } else {
      Err("No Type\n");
      throw;
    }
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
   * 先读后写寄存器
   * @tparam value 要写的值
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadWrite(uint64_t value) {
    auto old_value = ReadOnlyRegBase<Reg>::Read();
    WriteOnlyRegBase<Reg>::Write(value);
    return old_value;
  }

  /**
   * 通过偏移先读后写寄存器
   * @param offset 偏移
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadSetBits(uint64_t offset) {
    auto old_value = ReadOnlyRegBase<Reg>::Read();
    WriteOnlyRegBase<Reg>::SetBits(offset);
    return old_value;
  }

  /**
   * 通过偏移先读后清零寄存器
   * @param offset 偏移
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t ReadClearBits(uint64_t offset) {
    auto old_value = ReadOnlyRegBase<Reg>::Read();
    WriteOnlyRegBase<Reg>::ClearBits(offset);
    return old_value;
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
  static __always_inline void Set() { Reg::SetBits(Info::kBitOffset); }

  /**
   * 清零对应 Reg 的由 Info 规定的指定位
   */
  static __always_inline void Clear() { Reg::ClearBits(Info::kBitOffset); }
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

// 第三部分：寄存器实例
class Rbp : public ReadWriteRegBase<reginfo::RbpInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Rbp &rbp) {
    printf("val: 0x%p", (void *)rbp.Read());
    return os;
  }
};

class Efer : public ReadWriteRegBase<reginfo::EferInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::EferInfo>, reginfo::EferInfo::Sce>
      sce;
  ReadWriteField<ReadWriteRegBase<reginfo::EferInfo>, reginfo::EferInfo::Lme>
      lme;
  ReadWriteField<ReadWriteRegBase<reginfo::EferInfo>, reginfo::EferInfo::Lma>
      lma;
  ReadWriteField<ReadWriteRegBase<reginfo::EferInfo>, reginfo::EferInfo::Nxe>
      nxe;

  /// @name 构造/析构函数
  /// @{
  Efer() = default;
  Efer(const Efer &) = delete;
  Efer(Efer &&) = delete;
  auto operator=(const Efer &) -> Efer & = delete;
  auto operator=(Efer &&) -> Efer & = delete;
  virtual ~Efer() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Efer &efer) {
    auto sce = efer.sce.Get();
    auto lme = efer.lme.Get();
    auto lma = efer.lma.Get();
    auto nxe = efer.nxe.Get();
    printf("val: 0x%p, sce: %s, lme: %s, lma: %s, nxe: %s", (void *)efer.Read(),
           (sce == true ? "Enable" : "Disable"),
           (lme == true ? "Enable" : "Disable"),
           (lma == true ? "Enable" : "Disable"),
           (nxe == true ? "Enable" : "Disable")

    );
    return os;
  }
};

class Rflags : public ReadWriteRegBase<reginfo::RflagsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::RflagsInfo>, reginfo::RflagsInfo::If>
      interrupt_enable_flag;

  /// @name 构造/析构函数
  /// @{
  Rflags() = default;
  Rflags(const Rflags &) = delete;
  Rflags(Rflags &&) = delete;
  auto operator=(const Rflags &) -> Rflags & = delete;
  auto operator=(Rflags &&) -> Rflags & = delete;
  virtual ~Rflags() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Rflags &Rflags) {
    auto interrupt_enable_flag = Rflags.interrupt_enable_flag.Get();
    printf("val: 0x%p, if: %s, lme: %s, lma: %s, nxe: %s",
           (void *)Rflags.Read(),
           (interrupt_enable_flag == true ? "Enable" : "Disable"));
    return os;
  }
};

class Gdtr : public ReadWriteRegBase<reginfo::GdtrInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Gdtr &gdtr) {
    printf("val: 0x%p", (void *)gdtr.Read());
    return os;
  }
};

class Ldtr : public ReadWriteRegBase<reginfo::LdtrInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Ldtr &ldtr) {
    printf("val: 0x%p", (void *)ldtr.Read());
    return os;
  }
};

class Idtr : public ReadWriteRegBase<reginfo::IdtrInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Idtr &idtr) {
    printf("val: 0x%p", (void *)idtr.Read());
    return os;
  }
};

/// 通用寄存器
struct AllXreg {
  Rbp rbp;
};

struct AllCr {
  Efer efer;
};

};  // namespace

// 第四部分：访问接口
[[maybe_unused]] static AllXreg kAllXreg;
[[maybe_unused]] static AllCr kAllCr;

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_HPP_

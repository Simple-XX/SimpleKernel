
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
 * @see sdm.pdf#3.5.1
 * @see sdm.pdf#3.5.2
 */
struct GdtrInfo : public RegInfoBase {
  /**
   * 段描述符结构
   * @see sdm.pdf#3.4.5
   * @see sdm.pdf#5.2.1
   * @see sdm.pdf#5.3.1
   */
  class SegmentDescriptor {
   public:
    enum Type {
      kDataReadOnly = 0,
      kDataReadOnlyAccessed = 1,
      kDataReadWrite = 2,
      kDataReadWriteAccessed = 3,
      kDataReadOnlyExpandDown = 4,
      kDataReadOnlyExpandDownAccessed = 5,
      kDataReadWriteExpandDown = 6,
      kDataReadWriteExpandDownAccessed = 7,

      kCodeExecuteOnly = 8,
      kCodeExecuteOnlyAccessed = 9,
      kCodeExecuteRead = 10,
      kCodeExecuteReadAccessed = 11,
      kCodeExecuteOnlyConforming = 12,
      kCodeExecuteOnlyConformingAccessed = 13,
      kCodeExecuteReadConforming = 14,
      kCodeExecuteReadConformingAccessed = 15
    };

    enum S {
      kSystem = 0,
      kCodeData = 1,
    };

    enum DPL {
      kRing0 = 0,
      kRing1 = 1,
      kRing2 = 2,
      kRing3 = 3,
    };

    enum P {
      kNotPresent = 0,
      kPresent = 1,
    };

    enum AVL {
      kNotAvailable = 0,
      kAvailable = 1,
    };

    enum L {
      kLegacy = 0,
      k64Bit = 1,
    };

    union {
      struct {
        uint64_t unused1 : 40;
        /// Segment type
        uint64_t type : 4;
        /// Descriptor type
        uint64_t s : 1;
        /// Specifies the privilege level of the segment
        uint64_t dpl : 2;
        /// Indicates whether the segment is present in memory (set) or not
        /// present (clear).
        uint64_t p : 1;
        uint64_t unused2 : 4;
        /// Available for use by system software
        uint64_t avl : 1;
        /// 64-bit code segment (IA-32e mode only)
        uint64_t l : 1;
        uint64_t unused3 : 10;
      } segment_descriptor;
      uint64_t val;
    };

    explicit SegmentDescriptor(Type type, S s, DPL dpl, P p, AVL avl, L l)
        : val(0) {
      segment_descriptor.type = type;
      segment_descriptor.s = s;
      segment_descriptor.dpl = dpl;
      segment_descriptor.p = p;
      segment_descriptor.avl = avl;
      segment_descriptor.l = l;
    }

    /// @name 构造/析构函数
    /// @{
    SegmentDescriptor() = default;
    SegmentDescriptor(const SegmentDescriptor &) = delete;
    SegmentDescriptor(SegmentDescriptor &&) = delete;
    auto operator=(const SegmentDescriptor &) -> SegmentDescriptor & = delete;
    auto operator=(SegmentDescriptor &&) -> SegmentDescriptor & = delete;
    virtual ~SegmentDescriptor() = default;
    /// @}

    friend std::ostream &operator<<(
        std::ostream &os, const SegmentDescriptor &segment_descriptor) {
      printf("val: 0x%p, type: 0x%X, s: %s, dpl: 0x%X, p: %s, avl: %s, l: %s",
             (void *)segment_descriptor.val,
             segment_descriptor.segment_descriptor.type,
             (segment_descriptor.segment_descriptor.s == kSystem) ? "System"
                                                                  : "CodeData",
             segment_descriptor.segment_descriptor.dpl,
             segment_descriptor.segment_descriptor.p ? "Present" : "NotPresent",
             segment_descriptor.segment_descriptor.avl ? "Available"
                                                       : "NotAvailable",
             (segment_descriptor.segment_descriptor.l == kLegacy) ? "Legacy"
                                                                  : "64Bit");
      return os;
    }
  };

  /// gdt 数量
  static constexpr const uint16_t kGdtMaxCount = 8;

  struct Gdtr {
    /// 全局描述符表限长
    uint16_t limit;
    /// 全局描述符表基地址
    SegmentDescriptor *base;
  } __attribute__((packed));

  using DataType = Gdtr;

  struct Limit {
    using DataType = uint16_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 16;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Base {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 64;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

/**
 * @brief ldtr 寄存器
 * @see sdm.pdf#2.4.2
 * @see sdm.pdf#6.3.1
 */
struct LdtrInfo : public RegInfoBase {};

/**
 * @brief idtr 寄存器
 * @see sdm.pdf#2.4.3
 */
struct IdtrInfo : public RegInfoBase {
  /// 最大中断数
  static constexpr const uint32_t kInterruptMaxCount = 22;

  /// 中断名
  static constexpr const char *const kInterruptNames[kInterruptMaxCount] = {
      "Divide Error",
      "Debug Exception",
      "NMI Interrupt",
      "Breakpoint",
      "Overflow",
      "BOUND Range Exceeded",
      "Invalid Opcode (Undefined Opcode)",
      "Device Not Available (No Math Coprocessor)",
      "Double Fault",
      "Coprocessor Segment Overrun (reserved)",
      "Invalid TSS",
      "Segment Not Present",
      "Stack-Segment Fault",
      "General Protection",
      "Page Fault",
      "(Intel reserved. Do not use.)",
      "x87 FPU Floating-Point Error (Math Fault)",
      "Alignment Check",
      "Machine Check",
      "SIMD Floating-Point Exception",
      "Virtualization Exception",
      "Control Protection Exception",
  };

  /**
   * @brief idt 结构
   * @see sdm.pdf#6.14.1
   */
  struct Idt {
    union {
      struct {
        // 低位地址
        uint64_t offset1 : 16;
        // 段选择子
        uint64_t selector : 16;
        // 中断栈表
        uint64_t ist : 3;
        // 填充 0
        uint64_t zero0 : 5;
        // 类型
        uint64_t type : 4;
        // 填充 0
        uint64_t zero1 : 1;
        // 权限
        uint64_t dpl : 2;
        // 存在位
        uint64_t p : 1;
        // 中段地址
        uint64_t offset2 : 16;
        // 高位地址
        uint64_t offset3 : 32;
        // 保留
        uint64_t reserved : 32;
      } idt;
      uint64_t val[2];
    };
  } __attribute__((packed));

  /**
   * @brief idtr 结构
   * @see sdm.pdf#2.4
   */
  struct Idtr {
    uint16_t limit;
    Idt *base;
  } __attribute__((packed));

  using DataType = Idtr;
  static constexpr uint64_t kBitWidth = 80;

  struct Limit {
    using DataType = uint16_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 16;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Base {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 64;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

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

/**
 * @brief 段寄存器
 * @see sdm.pdf#3.4.3
 * @note In 64-bit mode the CS/SS/DS/ES segments are ignored and the base
 * address is always 0 to provide a full 64bit address space. The FS and GS
 * segments are still functional in 64-bit mode.
 */
namespace segment_register {
/**
 * @brief 段选择子
 * @see sdm.pdf#3.4.2
 */
struct SegmentSelector : public RegInfoBase {
  struct Rpl {
    using DataType = uint8_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 2;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Ti {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 2;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Index {
    using DataType = uint16_t;
    static constexpr uint64_t kBitOffset = 3;
    static constexpr uint64_t kBitWidth = 13;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

struct CsInfo : public RegInfoBase {
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
};

struct SsInfo : public RegInfoBase {
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
};

struct DsInfo : public RegInfoBase {
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
};

struct EsInfo : public RegInfoBase {
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
};

struct FsInfo : public RegInfoBase {
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
};

struct GsInfo : public RegInfoBase {
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
};

};  // namespace segment_register

};  // namespace reginfo

// 第二部分：读/写模版实现
namespace {
/**
 * 只读接口
 * @tparam 寄存器类型
 */
template <class RegInfo>
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
   * @return RegInfo::DataType 寄存器的值
   */
  static __always_inline RegInfo::DataType Read() {
    typename RegInfo::DataType value{};
    if constexpr (std::is_same<RegInfo, reginfo::RbpInfo>::value) {
      __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::EferInfo>::value) {
      uint32_t low{};
      uint32_t high{};
      __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000080) :);
      value = ((uint64_t)high << 32) | low;
    } else if constexpr (std::is_same<RegInfo, reginfo::RflagsInfo>::value) {
      __asm__ volatile("pushfq; popq %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::GdtrInfo>::value) {
      __asm__ volatile("sgdt %0" : "=m"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::LdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::IdtrInfo>::value) {
      __asm__ volatile("sidt %0" : "=m"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::TrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("mov %%cr0, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("mov %%cr2, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("mov %%cr3, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("mov %%cr4, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("mov %%cr8, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::CpuidInfo>::value) {
      __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::Xcr0Info>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::CsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::SsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::DsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::EsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::FsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::GsInfo>::value) {
      Err("TODO\n");
    } else {
      Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * () 重载
   */
  static __always_inline RegInfo::DataType operator()() { return Read(); }
};

/**
 * 只写接口
 * @tparam 寄存器类型
 */
template <class RegInfo>
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
  static __always_inline void Write(RegInfo::DataType value) {
    if constexpr (std::is_same<RegInfo, reginfo::RbpInfo>::value) {
      __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::EferInfo>::value) {
      uint32_t low = value & 0xFFFFFFFF;
      uint32_t high = value >> 32;
      __asm__ volatile("wrmsr" : : "c"(0xC0000080), "a"(low), "d"(high) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::RflagsInfo>::value) {
      __asm__ volatile("pushq %0; popfq" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::GdtrInfo>::value) {
      __asm__ volatile("lgdt %0" : : "m"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::LdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::IdtrInfo>::value) {
      __asm__ volatile("lidt %0" : : "m"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::TrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("mov %0, %%cr0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("mov %0, %%cr2" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("mov %0, %%cr3" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("mov %0, %%cr4" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("mov %0, %%cr8" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::Xcr0Info>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::CsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::SsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::DsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::EsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::FsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             reginfo::segment_register::GsInfo>::value) {
      Err("TODO\n");
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
    if constexpr (std::is_same<RegInfo, reginfo::RbpInfo>::value) {
      __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::EferInfo>::value) {
      uint32_t low{};
      uint32_t high{};
      __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000080) :);
      uint64_t value = ((uint64_t)high << 32) | low;
      value |= (1ULL << offset);
      Write(value);
    } else if constexpr (std::is_same<RegInfo, reginfo::RflagsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::GdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::LdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::IdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::TrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("bts %%cr0, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("bts %%cr2, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("bts %%cr3, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("bts %%cr4, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("bts %%cr8, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::Xcr0Info>::value) {
      Err("TODO\n");
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
    if constexpr (std::is_same<RegInfo, reginfo::RbpInfo>::value) {
      __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::EferInfo>::value) {
      uint32_t low{};
      uint32_t high{};
      __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000080) :);
      uint64_t value = ((uint64_t)high << 32) | low;
      value &= ~(1ULL << offset);
      Write(value);
    } else if constexpr (std::is_same<RegInfo, reginfo::RflagsInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::GdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::LdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::IdtrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::TrInfo>::value) {
      Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr0Info>::value) {
      __asm__ volatile("btr %%cr0, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr2Info>::value) {
      __asm__ volatile("btr %%cr2, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr3Info>::value) {
      __asm__ volatile("btr %%cr3, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr4Info>::value) {
      __asm__ volatile("btr %%cr4, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::cr::Cr8Info>::value) {
      __asm__ volatile("btr %%cr8, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::Xcr0Info>::value) {
      Err("TODO\n");
    } else {
      Err("No Type\n");
      throw;
    }
  }

  /**
   * |= 重载
   */
  __always_inline void operator|=(uint64_t offset) { SetBits(offset); }
};

/**
 * 读写接口
 * @tparam 寄存器类型
 */
template <class RegInfo>
class ReadWriteRegBase : public ReadOnlyRegBase<RegInfo>,
                         public WriteOnlyRegBase<RegInfo> {
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
   * @return RegInfo::DataType 寄存器的值
   */
  static __always_inline RegInfo::DataType ReadWrite(RegInfo::DataType value) {
    auto old_value = ReadOnlyRegBase<RegInfo>::Read();
    WriteOnlyRegBase<RegInfo>::Write(value);
    return old_value;
  }

  /**
   * 通过偏移先读后写寄存器
   * @param offset 偏移
   * @return RegInfo::DataType 寄存器的值
   */
  static __always_inline RegInfo::DataType ReadSetBits(uint64_t offset) {
    auto old_value = ReadOnlyRegBase<RegInfo>::Read();
    WriteOnlyRegBase<RegInfo>::SetBits(offset);
    return old_value;
  }

  /**
   * 通过偏移先读后清零寄存器
   * @param offset 偏移
   * @return RegInfo::DataType 寄存器的值
   */
  static __always_inline RegInfo::DataType ReadClearBits(uint64_t offset) {
    auto old_value = ReadOnlyRegBase<RegInfo>::Read();
    WriteOnlyRegBase<RegInfo>::ClearBits(offset);
    return old_value;
  }
};

/**
 * 只读位域接口
 * @tparam Reg 寄存器类型
 * @tparam RegInfo 寄存器数据信息
 */
template <class Reg, class RegInfo>
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
   * 获取对应 Reg 的由 RegInfo 规定的指定位的值
   * @return RegInfo::DataType 指定位值的信息
   */
  static __always_inline RegInfo::DataType Get() {
    if constexpr (std::is_same<RegInfo, reginfo::GdtrInfo::Limit>::value) {
      return Reg::Read().limit;
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::GdtrInfo::Base>::value) {
      return Reg::Read().base;
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::IdtrInfo::Limit>::value) {
      return Reg::Read().limit;
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::IdtrInfo::Base>::value) {
      return Reg::Read().base;
    } else {
      return (typename RegInfo::DataType)((Reg::Read() & RegInfo::kBitMask) >>
                                          RegInfo::kBitOffset);
    }
  }

  /**
   * 从指定的值获取对应 Reg 的由 RegInfo 规定的指定位的值
   * @param value 指定的值
   * @return RegInfo::DataType 指定位值的信息
   */
  static __always_inline RegInfo::DataType Get(RegInfo::DataType value) {
    if constexpr (std::is_same<RegInfo, reginfo::GdtrInfo::Limit>::value) {
      return value;
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::GdtrInfo::Base>::value) {
      return value;
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::IdtrInfo::Limit>::value) {
      return value;
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::IdtrInfo::Base>::value) {
      return value;
    } else {
      return (typename RegInfo::DataType)((value & RegInfo::kBitMask) >>
                                          RegInfo::kBitOffset);
    }
  }
};

/**
 * 只写位域接口
 * @tparam Reg 寄存器类型
 * @tparam RegInfo 寄存器数据信息
 */
template <class Reg, class RegInfo>
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
   * 置位对应 Reg 的由 RegInfo 规定的指定位
   */
  static __always_inline void Set() { Reg::SetBits(RegInfo::kBitOffset); }

  /**
   * 清零对应 Reg 的由 RegInfo 规定的指定位
   */
  static __always_inline void Clear() { Reg::ClearBits(RegInfo::kBitOffset); }
};

/**
 * 读写位域接口
 * @tparam Reg 寄存器类型
 * @tparam RegInfo 寄存器数据信息
 */
template <class Reg, class RegInfo>
class ReadWriteField : public ReadOnlyField<Reg, RegInfo>,
                       public WriteOnlyField<Reg, RegInfo> {
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
  static __always_inline void Write(RegInfo::DataType value) {
    auto org_value = Reg::Read();
    auto new_value = (org_value & ~RegInfo::kBitMask) |
                     ((value << RegInfo::kBitOffset) & RegInfo::kBitMask);
    Reg::Write(new_value);
  }

  /**
   * 先读出旧值，后将寄存器的值替换为指定值
   * @param value 新值
   * @return RegInfo::DataType 由寄存器规定的数据类型
   */
  static __always_inline RegInfo::DataType ReadWrite(RegInfo::DataType value) {
    auto org_value = Reg::Read();
    auto new_value = (org_value & ~RegInfo::kBitMask) |
                     ((value << RegInfo::kBitOffset) & RegInfo::kBitMask);
    Reg::Write(new_value);
    return (RegInfo::DataType)((org_value & RegInfo::kBitMask) >>
                               RegInfo::kBitOffset);
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
    printf("val: 0x%p, if: %s", (void *)Rflags.Read(),
           (interrupt_enable_flag == true ? "Enable" : "Disable"));
    return os;
  }
};

class Gdtr : public ReadWriteRegBase<reginfo::GdtrInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::GdtrInfo>, reginfo::GdtrInfo::Limit>
      limit;
  ReadWriteField<ReadWriteRegBase<reginfo::GdtrInfo>, reginfo::GdtrInfo::Base>
      base;

  friend std::ostream &operator<<(std::ostream &os, const Gdtr &gdtr) {
    printf("base: 0x%p, limit: %d", (void *)gdtr.Read().base,
           gdtr.Read().limit);
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
  ReadWriteField<ReadWriteRegBase<reginfo::IdtrInfo>, reginfo::IdtrInfo::Limit>
      limit;
  ReadWriteField<ReadWriteRegBase<reginfo::IdtrInfo>, reginfo::IdtrInfo::Base>
      base;

  friend std::ostream &operator<<(std::ostream &os, const Idtr &idtr) {
    printf("base: 0x%p, limit: %d", (void *)idtr.Read().base,
           idtr.Read().limit);
    return os;
  }
};

class Tr : public ReadWriteRegBase<reginfo::TrInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Tr &tr) {
    printf("val: 0x%p", (void *)tr.Read());
    return os;
  }
};

namespace cr {

class Cr0 : public ReadWriteRegBase<reginfo::cr::Cr0Info> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::cr::Cr0Info>,
                 reginfo::cr::Cr0Info::Pe>
      pe;
  ReadWriteField<ReadWriteRegBase<reginfo::cr::Cr0Info>,
                 reginfo::cr::Cr0Info::Pg>
      pg;

  /// @name 构造/析构函数
  /// @{
  Cr0() = default;
  Cr0(const Cr0 &) = delete;
  Cr0(Cr0 &&) = delete;
  auto operator=(const Cr0 &) -> Cr0 & = delete;
  auto operator=(Cr0 &&) -> Cr0 & = delete;
  virtual ~Cr0() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Cr0 &cr0) {
    auto pe = cr0.pe.Get();
    auto pg = cr0.pg.Get();
    printf("val: 0x%p, pe: %s, pg: %s", (void *)cr0.Read(),
           (pe == true ? "Enable" : "Disable"),
           (pg == true ? "Enable" : "Disable")

    );
    return os;
  }
};

class Cr2 : public ReadWriteRegBase<reginfo::cr::Cr2Info> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Cr2 &cr2) {
    printf("val: 0x%p", (void *)cr2.Read());
    return os;
  }
};

class Cr3 : public ReadWriteRegBase<reginfo::cr::Cr3Info> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::cr::Cr3Info>,
                 reginfo::cr::Cr3Info::Pwt>
      pwt;
  ReadWriteField<ReadWriteRegBase<reginfo::cr::Cr3Info>,
                 reginfo::cr::Cr3Info::Pcd>
      pcd;
  ReadWriteField<ReadWriteRegBase<reginfo::cr::Cr3Info>,
                 reginfo::cr::Cr3Info::PageDirectoryBase>
      page_directory_base;

  /// @name 构造/析构函数
  /// @{
  Cr3() = default;
  Cr3(const Cr3 &) = delete;
  Cr3(Cr3 &&) = delete;
  auto operator=(const Cr3 &) -> Cr3 & = delete;
  auto operator=(Cr3 &&) -> Cr3 & = delete;
  virtual ~Cr3() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Cr3 &cr3) {
    auto pwt = cr3.pwt.Get();
    auto pcd = cr3.pcd.Get();
    auto page_directory_base = cr3.page_directory_base.Get();
    printf("val: 0x%p, pwt: %s, pcd: %s, page_directory_base: 0x%p",
           (void *)cr3.Read(), (pwt == true ? "Enable" : "Disable"),
           (pcd == true ? "Enable" : "Disable"), (void *)page_directory_base);
    return os;
  }
};

class Cr4 : public ReadWriteRegBase<reginfo::cr::Cr4Info> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::cr::Cr4Info>,
                 reginfo::cr::Cr4Info::Pae>
      pae;

  /// @name 构造/析构函数
  /// @{
  Cr4() = default;
  Cr4(const Cr4 &) = delete;
  Cr4(Cr4 &&) = delete;
  auto operator=(const Cr4 &) -> Cr4 & = delete;
  auto operator=(Cr4 &&) -> Cr4 & = delete;
  virtual ~Cr4() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Cr4 &cr4) {
    auto pae = cr4.pae.Get();
    printf("val: 0x%p, pae: %s", (void *)cr4.Read(),
           (pae == true ? "Enable" : "Disable"));
    return os;
  }
};

class Cr8 : public ReadWriteRegBase<reginfo::cr::Cr8Info> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Cr8 &cr8) {
    printf("val: 0x%p", (void *)cr8.Read());
    return os;
  }
};

};  // namespace cr

class Cpuid : public ReadOnlyRegBase<reginfo::CpuidInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Cpuid &cpuid) {
    printf("val: 0x%p", (void *)cpuid.Read());
    return os;
  }
};

class Xcr0 : public ReadWriteRegBase<reginfo::Xcr0Info> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Xcr0 &xcr0) {
    printf("val: 0x%p", (void *)xcr0.Read());
    return os;
  }
};

namespace segment_register {
class Cs : public ReadWriteRegBase<reginfo::segment_register::CsInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Cs &cs) {
    printf("val: 0x%X", cs.Read());
    return os;
  }
};

class Ss : public ReadWriteRegBase<reginfo::segment_register::SsInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Ss &ss) {
    printf("val: 0x%X", ss.Read());
    return os;
  }
};

class Ds : public ReadWriteRegBase<reginfo::segment_register::DsInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Ds &ds) {
    printf("val: 0x%X", ds.Read());
    return os;
  }
};

class Es : public ReadWriteRegBase<reginfo::segment_register::EsInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Es &es) {
    printf("val: 0x%X", es.Read());
    return os;
  }
};

class Fs : public ReadWriteRegBase<reginfo::segment_register::FsInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Fs &fs) {
    printf("val: 0x%X", fs.Read());
    return os;
  }
};

class Gs : public ReadWriteRegBase<reginfo::segment_register::GsInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Gs &gs) {
    printf("val: 0x%X", gs.Read());
    return os;
  }
};
};  // namespace segment_register

/// 通用寄存器
struct AllXreg {
  Rbp rbp;
};

struct AllCr {
  Efer efer;
  Rflags rflags;
  Gdtr gdtr;
  Ldtr ldtr;
  Idtr idtr;
  Tr tr;
  cr::Cr0 cr0;
  cr::Cr2 cr2;
  cr::Cr3 cr3;
  cr::Cr4 cr4;
  cr::Cr8 cr8;
  Cpuid cpuid;
  Xcr0 xcr0;
  segment_register::Cs cs;
  segment_register::Ss ss;
  segment_register::Ds ds;
  segment_register::Es es;
  segment_register::Fs fs;
  segment_register::Gs gs;
};

};  // namespace

// 第四部分：访问接口
[[maybe_unused]] static AllXreg kAllXreg;
[[maybe_unused]] static AllCr kAllCr;

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_HPP_

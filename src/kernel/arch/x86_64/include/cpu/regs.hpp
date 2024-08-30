
/**
 * @file regs.hpp
 * @brief x86_64 寄存器相关定义
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_REGS_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_REGS_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "kernel_log.hpp"
#include "sk_cstdio"
#include "sk_iostream"

/**
 * x86_64 cpu Control Registers 相关定义
 * @note 寄存器读写设计见 arch/README.md
 */
namespace cpu {

// 第一部分：寄存器定义
namespace register_info {

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
 * @brief 系统段与门描述符类型
 * @see sdm.pdf#3.5
 */
enum SystemSegmentAndGateDescriptorTypes {
  kReserved0 = 0,
  k16BitTssAvailable = 1,
  kLdt = 2,
  k16BitTssBusy = 3,
  k16BitCallGate = 4,
  kTaskGate = 5,
  k16BitInterruptGate = 6,
  k16BitTrapGate = 7,
  kReserved8 = 8,
  k32BitTssAvailable = 9,
  k64BitTssAvailable = k32BitTssAvailable,
  kReserved10 = 10,
  k32BitTssBusy = 11,
  k64BitTssBusy = k32BitTssBusy,
  k32BitCallGate = 12,
  k64BitCallGate = k32BitCallGate,
  kReserved13 = 13,
  k32BitInterruptGate = 14,
  k64BitInterruptGate = k32BitInterruptGate,
  k32BitTrapGate = 15,
  k64BitTrapGate = k32BitTrapGate,
};

/// 描述符权限级别
enum DescriptorDpl {
  kRing0 = 0,
  kRing1 = 1,
  kRing2 = 2,
  kRing3 = 3,
};

/// 描述符存在位
enum DescriptorP {
  kNotPresent = 0,
  kPresent = 1,
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

    using DPL = DescriptorDpl;
    using P = DescriptorP;

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
        uint64_t limit_low : 16;
        uint64_t base_low : 16;
        uint64_t base_mid : 8;
        /// Segment type
        uint64_t type : 4;
        /// Descriptor type
        uint64_t s : 1;
        /// Specifies the privilege level of the segment
        uint64_t dpl : 2;
        /// Indicates whether the segment is present in memory (set) or not
        /// present (clear).
        uint64_t p : 1;
        uint64_t limit_high : 4;
        /// Available for use by system software
        uint64_t avl : 1;
        /// 64-bit code segment (IA-32e mode only)
        uint64_t l : 1;
        uint64_t db : 1;
        uint64_t g : 1;
        uint64_t base_high : 8;
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
    SegmentDescriptor(const SegmentDescriptor &) = default;
    SegmentDescriptor(SegmentDescriptor &&) = default;
    auto operator=(const SegmentDescriptor &) -> SegmentDescriptor & = default;
    auto operator=(SegmentDescriptor &&) -> SegmentDescriptor & = default;
    ~SegmentDescriptor() = default;
    /// @}

    friend sk_std::ostream &operator<<(
        sk_std::ostream &os, const SegmentDescriptor &segment_descriptor) {
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
  static constexpr const uint16_t kMaxCount = 5;
  /// 首个空描述符索引
  static constexpr const uint16_t kNullIndex = 0;
  /// 内核代码段描述符索引
  static constexpr const uint16_t kKernelCodeIndex = 1;
  /// 内核数据段描述符索引
  static constexpr const uint16_t kKernelDataIndex = 2;
  /// 用户代码段描述符索引
  static constexpr const uint16_t kUserCodeIndex = 3;
  /// 用户数据段描述符索引
  static constexpr const uint16_t kUserDataIndex = 4;

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
    using DataType = SegmentDescriptor *;
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
  static constexpr const uint32_t kInterruptMaxCount = 256;

  /// 中断号
  /// @see sdm.pdf#6.3.1
  enum {
    kDivideError = 0,
    kDebugException = 1,
    kNmiInterrupt = 2,
    kBreakpoint = 3,
    kOverflow = 4,
    kBoundRangeExceeded = 5,
    kInvalidOpcode = 6,
    kDeviceNotAvailable = 7,
    kDoubleFault = 8,
    // 386 后不再使用
    kCoprocessorSegmentOverrun = 9,
    kInvalidTss = 10,
    kSegmentNotPresent = 11,
    kStackSegmentFault = 12,
    kGeneralProtection = 13,
    kPageFault = 14,
    kX87FpuFloatingPointError = 16,
    kAlignmentCheck = 17,
    kMachineCheck = 18,
    kSIMDFloatingPointException = 19,
    kVirtualizationException = 20,
    ControlProtectionException = 21,

    /// @todo
    /// 电脑系统计时器
    kIrq0 = 32,
    /// 键盘
    kIrq1 = 33,
    /// 与 IRQ9 相接，MPU-401 MD 使用
    kIrq2 = 34,
    /// 串口设备
    kIrq3 = 35,
    /// 串口设备
    kIrq4 = 36,
    /// 建议声卡使用
    kIrq5 = 37,
    /// 软驱传输控制使用
    kIrq6 = 38,
    /// 打印机传输控制使用
    kIrq7 = 39,
    /// 即时时钟
    kIrq8 = 40,
    /// 与 IRQ2 相接，可设定给其他硬件
    kIrq9 = 41,
    /// 建议网卡使用
    kIrq10 = 42,
    /// 建议 AGP 显卡使用
    kIrq11 = 43,
    /// 接 PS/2 鼠标，也可设定给其他硬件
    kIrq12 = 44,
    /// 协处理器使用
    kIrq13 = 45,
    /// SATA 主硬盘
    kIrq14 = 46,
    /// SATA 从硬盘
    kIrq15 = 47,
    /// 系统调用
    kIrq128 = 128,
  };

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
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Reserved",
      "Irq0",
      "Irq1",
      "Irq2",
      "Irq3",
      "Irq4",
      "Irq5",
      "Irq6",
      "Irq7",
      "Irq8",
      "Irq9",
      "Irq10",
      "Irq11",
      "Irq12",
      "Irq13",
      "Irq14",
      "Irq15",
  };

  /**
   * @brief 错误码结构
   * @see sdm.pdf#6.13
   */
  struct ErrorCode {
    union {
      struct {
        // When set, indicates that the exception occurred during
        // delivery of an event external to the program, such as an interrupt or
        // an earlier exception.1 The bit is cleared if the exception occurred
        // during delivery of a software interrupt (INT n, INT3, or INTO).
        uint32_t ext : 1;
        // When set, indicates that the index portion of the error code refers
        // to a gate descriptor in the IDT; when clear, indicates that the index
        // refers to a descriptor in the GDT or the current LDT.
        uint32_t idt : 1;
        // Only used when the IDT flag is clear. When set, the TI flag indicates
        // that the index portion of the error code refers to a segment or gate
        // descriptor in the LDT; when clear, it indi- cates that the index
        // refers to a descriptor in the current GDT.
        uint32_t ti : 1;
        // The segment selector index field provides an index into the IDT, GDT,
        // or current LDT to the segment or gate selector being referenced by
        // the error code.
        uint32_t segment_selector_index : 29;
      } error_code;
      uint32_t val;
    };

    friend sk_std::ostream &operator<<(sk_std::ostream &os,
                                       const ErrorCode &error_code) {
      printf(
          "val: 0x%X, ext: %d, idt: %d, ti: %d, segment_selector_index: "
          "0x%X",
          error_code.val, error_code.error_code.ext, error_code.error_code.idt,
          error_code.error_code.ti,
          error_code.error_code.segment_selector_index);
      return os;
    }
  };

  /**
   * @brief idt 结构
   * @see sdm.pdf#6.14.1
   */
  class Idt {
   public:
    using Type = SystemSegmentAndGateDescriptorTypes;
    using DPL = DescriptorDpl;
    using P = DescriptorP;

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

    explicit Idt(uint64_t base, uint16_t selector, uint8_t ist, uint8_t type,
                 uint8_t dpl, uint8_t p)
        : val{} {
      idt.offset1 = base & 0xFFFF;
      idt.selector = selector;
      idt.ist = ist;
      idt.type = type;
      idt.dpl = dpl;
      idt.p = p;
      idt.offset2 = (base >> 16) & 0xFFFF;
      idt.offset3 = base >> 32;
    }

    /// @name 构造/析构函数
    /// @{
    Idt() = default;
    Idt(const Idt &) = default;
    Idt(Idt &&) = default;
    auto operator=(const Idt &) -> Idt & = default;
    auto operator=(Idt &&) -> Idt & = default;
    ~Idt() = default;
    /// @}

    friend sk_std::ostream &operator<<(sk_std::ostream &os, const Idt &idt) {
      printf(
          "val: 0x%p 0x%p, offset: 0x%p, selector: 0x%X, type: %d, dpl: 0x%X, "
          "p: %s",
          (void *)idt.val[0], (void *)idt.val[1],
          (void *)(((uint64_t)idt.idt.offset3 << 32) |
                   ((uint64_t)idt.idt.offset2 << 16) |
                   (uint64_t)idt.idt.offset1),
          idt.idt.selector, idt.idt.type, idt.idt.dpl,
          idt.idt.p ? "Present" : "NotPresent");
      return os;
    }
  };

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
  using DataType = uint16_t;
  static constexpr uint64_t kBitWidth = 16;
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

/// 无法直接写入，通常是在调用远跳转、调用或返回时更改
struct CsInfo : public SegmentSelector {};

struct SsInfo : public SegmentSelector {};

struct DsInfo : public SegmentSelector {};

struct EsInfo : public SegmentSelector {};

struct FsInfo : public SegmentSelector {};

struct GsInfo : public SegmentSelector {};

};  // namespace segment_register

};  // namespace register_info

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
    if constexpr (std::is_same<RegInfo, register_info::RbpInfo>::value) {
      __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::EferInfo>::value) {
      uint32_t low{};
      uint32_t high{};
      __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000080) :);
      value = ((uint64_t)high << 32) | low;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::RflagsInfo>::value) {
      __asm__ volatile("pushfq; popq %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::GdtrInfo>::value) {
      __asm__ volatile("sgdt %0" : "=m"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::LdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo>::value) {
      __asm__ volatile("sidt %0" : "=m"(value) : :);
    } else if constexpr (std::is_same<RegInfo, register_info::TrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr0Info>::value) {
      __asm__ volatile("mov %%cr0, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr2Info>::value) {
      __asm__ volatile("mov %%cr2, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr3Info>::value) {
      __asm__ volatile("mov %%cr3, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr4Info>::value) {
      __asm__ volatile("mov %%cr4, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr8Info>::value) {
      __asm__ volatile("mov %%cr8, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::CpuidInfo>::value) {
      __asm__ volatile("mov %%rbp, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::Xcr0Info>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::CsInfo>::value) {
      __asm__ volatile("mov %%cs, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::SsInfo>::value) {
      __asm__ volatile("mov %%ss, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::DsInfo>::value) {
      __asm__ volatile("mov %%ds, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::EsInfo>::value) {
      __asm__ volatile("mov %%es, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::FsInfo>::value) {
      __asm__ volatile("mov %%fs, %0" : "=r"(value) : :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::GsInfo>::value) {
      __asm__ volatile("mov %%gs, %0" : "=r"(value) : :);
    } else {
      klog::Err("No Type\n");
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
    if constexpr (std::is_same<RegInfo, register_info::RbpInfo>::value) {
      __asm__ volatile("mov %0, %%rbp" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::EferInfo>::value) {
      uint32_t low = value & 0xFFFFFFFF;
      uint32_t high = value >> 32;
      __asm__ volatile("wrmsr" : : "c"(0xC0000080), "a"(low), "d"(high) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::RflagsInfo>::value) {
      __asm__ volatile("pushq %0; popfq" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::GdtrInfo>::value) {
      __asm__ volatile("lgdt %0" : : "m"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::LdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo>::value) {
      __asm__ volatile("lidt %0" : : "m"(value) :);
    } else if constexpr (std::is_same<RegInfo, register_info::TrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr0Info>::value) {
      __asm__ volatile("mov %0, %%cr0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr2Info>::value) {
      __asm__ volatile("mov %0, %%cr2" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr3Info>::value) {
      __asm__ volatile("mov %0, %%cr3" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr4Info>::value) {
      __asm__ volatile("mov %0, %%cr4" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr8Info>::value) {
      __asm__ volatile("mov %0, %%cr8" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::Xcr0Info>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::CsInfo>::value) {
      auto JumpFunction = [=](uint16_t value) {
        __asm__ volatile(
            "push %0\n\t"
            "pushq $WriteCsLabel\n\t"
            "retfq\n\t"
            "WriteCsLabel: \n\t"
            :
            : "r"(value)
            :);
      };
      JumpFunction(value);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::SsInfo>::value) {
      __asm__ volatile("mov %0, %%ss" : : "r"(value) :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::DsInfo>::value) {
      __asm__ volatile("mov %0, %%ds" : : "r"(value) :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::EsInfo>::value) {
      __asm__ volatile("mov %0, %%es" : : "r"(value) :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::FsInfo>::value) {
      __asm__ volatile("mov %0, %%fs" : : "r"(value) :);
    } else if constexpr (std::is_same<
                             RegInfo,
                             register_info::segment_register::GsInfo>::value) {
      __asm__ volatile("mov %0, %%gs" : : "r"(value) :);
    } else {
      klog::Err("No Type\n");
      throw;
    }
  }

  /**
   * 通过偏移设置寄存器
   * @param offset 位偏移
   */
  static __always_inline void SetBits(uint64_t offset) {
    if constexpr (std::is_same<RegInfo, register_info::RbpInfo>::value) {
      __asm__ volatile("bts %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::EferInfo>::value) {
      uint32_t low{};
      uint32_t high{};
      __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000080) :);
      uint64_t value = ((uint64_t)high << 32) | low;
      value |= (1ULL << offset);
      Write(value);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::RflagsInfo>::value) {
      if (offset == register_info::RflagsInfo::If::kBitOffset) {
        __asm__ volatile("sti");
      } else {
        typename RegInfo::DataType old_value = 0;
        __asm__ volatile("pushfq; popq %0" : "=r"(old_value) : :);
        auto new_value = old_value | (1 << offset);
        Write(new_value);
      }
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::GdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::LdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, register_info::TrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr0Info>::value) {
      // __asm__ volatile("bts %%cr0, %0" : : "r"(offset) :);
      uint64_t value;
      __asm__ volatile(
          "mov %%cr0, %0\n\t"
          "bts %1, %0\n\t"
          "mov %0, %%cr0"
          : "=r"(value)
          : "r"(offset)
          : "memory");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr2Info>::value) {
      __asm__ volatile("bts %%cr2, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr3Info>::value) {
      __asm__ volatile("bts %%cr3, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr4Info>::value) {
      __asm__ volatile("bts %%cr4, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr8Info>::value) {
      __asm__ volatile("bts %%cr8, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::Xcr0Info>::value) {
      klog::Err("TODO\n");
    } else {
      klog::Err("No Type\n");
      throw;
    }
  }

  /**
   * 清零寄存器
   * @param offset 位偏移
   */
  static __always_inline void ClearBits(uint64_t offset) {
    if constexpr (std::is_same<RegInfo, register_info::RbpInfo>::value) {
      __asm__ volatile("btr %%rbp, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::EferInfo>::value) {
      uint32_t low{};
      uint32_t high{};
      __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000080) :);
      uint64_t value = ((uint64_t)high << 32) | low;
      value &= ~(1ULL << offset);
      Write(value);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::RflagsInfo>::value) {
      if (offset == register_info::RflagsInfo::If::kBitOffset) {
        __asm__ volatile("cli");
      } else {
        typename RegInfo::DataType old_value = 0;
        __asm__ volatile("pushfq; popq %0" : "=r"(old_value) : :);
        auto new_value = old_value & (~(1ULL << offset));
        Write(new_value);
      }
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::GdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::LdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo, register_info::TrInfo>::value) {
      klog::Err("TODO\n");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr0Info>::value) {
      // __asm__ volatile("btr %%cr0, %0" : : "r"(offset) :);
      // __asm__ volatile("bts %%cr0, %0" : : "r"(offset) :);
      uint64_t value;
      __asm__ volatile(
          "mov %%cr0, %0\n\t"
          "btr %1, %0\n\t"
          "mov %0, %%cr0"
          : "=r"(value)
          : "r"(offset)
          : "memory");
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr2Info>::value) {
      __asm__ volatile("btr %%cr2, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr3Info>::value) {
      __asm__ volatile("btr %%cr3, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr4Info>::value) {
      __asm__ volatile("btr %%cr4, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::cr::Cr8Info>::value) {
      __asm__ volatile("btr %%cr8, %0" : : "r"(offset) :);
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::Xcr0Info>::value) {
      klog::Err("TODO\n");
    } else {
      klog::Err("No Type\n");
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
    if constexpr (std::is_same<RegInfo,
                               register_info::GdtrInfo::Limit>::value) {
      return Reg::Read().limit;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::GdtrInfo::Base>::value) {
      return Reg::Read().base;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo::Limit>::value) {
      return Reg::Read().limit;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo::Base>::value) {
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
    if constexpr (std::is_same<RegInfo,
                               register_info::GdtrInfo::Limit>::value) {
      return value;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::GdtrInfo::Base>::value) {
      return value;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo::Limit>::value) {
      return value;
    } else if constexpr (std::is_same<RegInfo,
                                      register_info::IdtrInfo::Base>::value) {
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
class Rbp : public ReadWriteRegBase<register_info::RbpInfo> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Rbp &rbp) {
    printf("val: 0x%p", (void *)rbp.Read());
    return os;
  }
};

class Efer : public ReadWriteRegBase<register_info::EferInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::EferInfo>,
                 register_info::EferInfo::Sce>
      sce;
  ReadWriteField<ReadWriteRegBase<register_info::EferInfo>,
                 register_info::EferInfo::Lme>
      lme;
  ReadWriteField<ReadWriteRegBase<register_info::EferInfo>,
                 register_info::EferInfo::Lma>
      lma;
  ReadWriteField<ReadWriteRegBase<register_info::EferInfo>,
                 register_info::EferInfo::Nxe>
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

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Efer &efer) {
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

class Rflags : public ReadWriteRegBase<register_info::RflagsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::RflagsInfo>,
                 register_info::RflagsInfo::If>
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

  friend sk_std::ostream &operator<<(sk_std::ostream &os,
                                     const Rflags &Rflags) {
    auto interrupt_enable_flag = Rflags.interrupt_enable_flag.Get();
    printf("val: 0x%p, if: %s", (void *)Rflags.Read(),
           (interrupt_enable_flag == true ? "Enable" : "Disable"));
    return os;
  }
};

class Gdtr : public ReadWriteRegBase<register_info::GdtrInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::GdtrInfo>,
                 register_info::GdtrInfo::Limit>
      limit;
  ReadWriteField<ReadWriteRegBase<register_info::GdtrInfo>,
                 register_info::GdtrInfo::Base>
      base;

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Gdtr &gdtr) {
    printf("base: 0x%p, limit: %d", (void *)gdtr.Read().base,
           gdtr.Read().limit);
    return os;
  }
};

class Ldtr : public ReadWriteRegBase<register_info::LdtrInfo> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Ldtr &ldtr) {
    printf("val: 0x%p", (void *)ldtr.Read());
    return os;
  }
};

class Idtr : public ReadWriteRegBase<register_info::IdtrInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::IdtrInfo>,
                 register_info::IdtrInfo::Limit>
      limit;
  ReadWriteField<ReadWriteRegBase<register_info::IdtrInfo>,
                 register_info::IdtrInfo::Base>
      base;

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Idtr &idtr) {
    printf("base: 0x%p, limit: %d", (void *)idtr.Read().base,
           idtr.Read().limit);
    return os;
  }
};

class Tr : public ReadWriteRegBase<register_info::TrInfo> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Tr &tr) {
    printf("val: 0x%p", (void *)tr.Read());
    return os;
  }
};

namespace cr {

class Cr0 : public ReadWriteRegBase<register_info::cr::Cr0Info> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::cr::Cr0Info>,
                 register_info::cr::Cr0Info::Pe>
      pe;
  ReadWriteField<ReadWriteRegBase<register_info::cr::Cr0Info>,
                 register_info::cr::Cr0Info::Pg>
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

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cr0 &cr0) {
    auto pe = cr0.pe.Get();
    auto pg = cr0.pg.Get();
    printf("val: 0x%p, pe: %s, pg: %s", (void *)cr0.Read(),
           (pe == true ? "Enable" : "Disable"),
           (pg == true ? "Enable" : "Disable")

    );
    return os;
  }
};

class Cr2 : public ReadWriteRegBase<register_info::cr::Cr2Info> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cr2 &cr2) {
    printf("val: 0x%p", (void *)cr2.Read());
    return os;
  }
};

class Cr3 : public ReadWriteRegBase<register_info::cr::Cr3Info> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::cr::Cr3Info>,
                 register_info::cr::Cr3Info::Pwt>
      pwt;
  ReadWriteField<ReadWriteRegBase<register_info::cr::Cr3Info>,
                 register_info::cr::Cr3Info::Pcd>
      pcd;
  ReadWriteField<ReadWriteRegBase<register_info::cr::Cr3Info>,
                 register_info::cr::Cr3Info::PageDirectoryBase>
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

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cr3 &cr3) {
    auto pwt = cr3.pwt.Get();
    auto pcd = cr3.pcd.Get();
    auto page_directory_base = cr3.page_directory_base.Get();
    printf("val: 0x%p, pwt: %s, pcd: %s, page_directory_base: 0x%p",
           (void *)cr3.Read(), (pwt == true ? "Enable" : "Disable"),
           (pcd == true ? "Enable" : "Disable"), (void *)page_directory_base);
    return os;
  }
};

class Cr4 : public ReadWriteRegBase<register_info::cr::Cr4Info> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::cr::Cr4Info>,
                 register_info::cr::Cr4Info::Pae>
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

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cr4 &cr4) {
    auto pae = cr4.pae.Get();
    printf("val: 0x%p, pae: %s", (void *)cr4.Read(),
           (pae == true ? "Enable" : "Disable"));
    return os;
  }
};

class Cr8 : public ReadWriteRegBase<register_info::cr::Cr8Info> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cr8 &cr8) {
    printf("val: 0x%p", (void *)cr8.Read());
    return os;
  }
};

};  // namespace cr

class Cpuid : public ReadOnlyRegBase<register_info::CpuidInfo> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cpuid &cpuid) {
    printf("val: 0x%p", (void *)cpuid.Read());
    return os;
  }
};

class Xcr0 : public ReadWriteRegBase<register_info::Xcr0Info> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Xcr0 &xcr0) {
    printf("val: 0x%p", (void *)xcr0.Read());
    return os;
  }
};

namespace segment_register {
class Cs : public ReadWriteRegBase<register_info::segment_register::CsInfo> {
 public:
  ReadOnlyField<ReadWriteRegBase<register_info::segment_register::CsInfo>,
                register_info::segment_register::CsInfo::Rpl>
      rpl;

  ReadOnlyField<ReadWriteRegBase<register_info::segment_register::CsInfo>,
                register_info::segment_register::CsInfo::Ti>
      ti;

  ReadOnlyField<ReadWriteRegBase<register_info::segment_register::CsInfo>,
                register_info::segment_register::CsInfo::Index>
      index;

  /// @name 构造/析构函数
  /// @{
  Cs() = default;
  Cs(const Cs &) = delete;
  Cs(Cs &&) = delete;
  auto operator=(const Cs &) -> Cs & = delete;
  auto operator=(Cs &&) -> Cs & = delete;
  virtual ~Cs() = default;
  /// @}

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Cs &cs) {
    auto val = cs.Read();
    auto rpl = cs.rpl.Get();
    auto ti = cs.ti.Get();
    auto index = cs.index.Get();
    printf("val: 0x%X, rpl: %d, ti: %s, index: 0x%X", val, rpl,
           ti ? "LDT" : "GDT", index);
    return os;
  }
};

class Ss : public ReadWriteRegBase<register_info::segment_register::SsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::segment_register::SsInfo>,
                 register_info::segment_register::SsInfo::Rpl>
      rpl;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::SsInfo>,
                 register_info::segment_register::SsInfo::Ti>
      ti;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::SsInfo>,
                 register_info::segment_register::SsInfo::Index>
      index;

  /// @name 构造/析构函数
  /// @{
  Ss() = default;
  Ss(const Ss &) = delete;
  Ss(Ss &&) = delete;
  auto operator=(const Ss &) -> Ss & = delete;
  auto operator=(Ss &&) -> Ss & = delete;
  virtual ~Ss() = default;
  /// @}

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Ss &ss) {
    auto val = ss.Read();
    auto rpl = ss.rpl.Get();
    auto ti = ss.ti.Get();
    auto index = ss.index.Get();
    printf("val: 0x%X, rpl: %d, ti: %s, index: 0x%X", val, rpl,
           ti ? "LDT" : "GDT", index);
    return os;
  }
};

class Ds : public ReadWriteRegBase<register_info::segment_register::DsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::segment_register::DsInfo>,
                 register_info::segment_register::DsInfo::Rpl>
      rpl;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::DsInfo>,
                 register_info::segment_register::DsInfo::Ti>
      ti;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::DsInfo>,
                 register_info::segment_register::DsInfo::Index>
      index;

  /// @name 构造/析构函数
  /// @{
  Ds() = default;
  Ds(const Ds &) = delete;
  Ds(Ds &&) = delete;
  auto operator=(const Ds &) -> Ds & = delete;
  auto operator=(Ds &&) -> Ds & = delete;
  virtual ~Ds() = default;
  /// @}

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Ds &ds) {
    auto val = ds.Read();
    auto rpl = ds.rpl.Get();
    auto ti = ds.ti.Get();
    auto index = ds.index.Get();
    printf("val: 0x%X, rpl: %d, ti: %s, index: 0x%X", val, rpl,
           ti ? "LDT" : "GDT", index);
    return os;
  }
};

class Es : public ReadWriteRegBase<register_info::segment_register::EsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::segment_register::EsInfo>,
                 register_info::segment_register::EsInfo::Rpl>
      rpl;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::EsInfo>,
                 register_info::segment_register::EsInfo::Ti>
      ti;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::EsInfo>,
                 register_info::segment_register::EsInfo::Index>
      index;

  /// @name 构造/析构函数
  /// @{
  Es() = default;
  Es(const Es &) = delete;
  Es(Es &&) = delete;
  auto operator=(const Es &) -> Es & = delete;
  auto operator=(Es &&) -> Es & = delete;
  virtual ~Es() = default;
  /// @}

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Es &es) {
    auto val = es.Read();
    auto rpl = es.rpl.Get();
    auto ti = es.ti.Get();
    auto index = es.index.Get();
    printf("val: 0x%X, rpl: %d, ti: %s, index: 0x%X", val, rpl,
           ti ? "LDT" : "GDT", index);
    return os;
  }
};

class Fs : public ReadWriteRegBase<register_info::segment_register::FsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::segment_register::FsInfo>,
                 register_info::segment_register::FsInfo::Rpl>
      rpl;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::FsInfo>,
                 register_info::segment_register::FsInfo::Ti>
      ti;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::FsInfo>,
                 register_info::segment_register::FsInfo::Index>
      index;

  /// @name 构造/析构函数
  /// @{
  Fs() = default;
  Fs(const Fs &) = delete;
  Fs(Fs &&) = delete;
  auto operator=(const Fs &) -> Fs & = delete;
  auto operator=(Fs &&) -> Fs & = delete;
  virtual ~Fs() = default;
  /// @}

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Fs &fs) {
    auto val = fs.Read();
    auto rpl = fs.rpl.Get();
    auto ti = fs.ti.Get();
    auto index = fs.index.Get();
    printf("val: 0x%X, rpl: %d, ti: %s, index: 0x%X", val, rpl,
           ti ? "LDT" : "GDT", index);
    return os;
  }
};

class Gs : public ReadWriteRegBase<register_info::segment_register::GsInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<register_info::segment_register::GsInfo>,
                 register_info::segment_register::GsInfo::Rpl>
      rpl;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::GsInfo>,
                 register_info::segment_register::GsInfo::Ti>
      ti;

  ReadWriteField<ReadWriteRegBase<register_info::segment_register::GsInfo>,
                 register_info::segment_register::GsInfo::Index>
      index;

  /// @name 构造/析构函数
  /// @{
  Gs() = default;
  Gs(const Gs &) = delete;
  Gs(Gs &&) = delete;
  auto operator=(const Gs &) -> Gs & = delete;
  auto operator=(Gs &&) -> Gs & = delete;
  virtual ~Gs() = default;
  /// @}

  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Gs &gs) {
    auto val = gs.Read();
    auto rpl = gs.rpl.Get();
    auto ti = gs.ti.Get();
    auto index = gs.index.Get();
    printf("val: 0x%X, rpl: %d, ti: %s, index: 0x%X", val, rpl,
           ti ? "LDT" : "GDT", index);
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

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_X86_64_INCLUDE_CPU_REGS_HPP_

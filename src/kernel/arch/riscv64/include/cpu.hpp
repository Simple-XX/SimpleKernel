
/**
 * @file cpu.hpp
 * @brief riscv64 cpu 相关定义
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "cstdio"
#include "iostream"
#include "kernel_log.hpp"

/**
 * riscv64 cpu 相关定义
 * @note 寄存器读写设计见 arch/README.md
 * @see priv-isa.pdf
 * https://github.com/riscv/riscv-isa-manual/releases/tag/20240411/priv-isa-asciidoc.pdf
 * @see riscv-abi.pdf
 * https://github.com/riscv-non-isa/riscv-elf-psabi-doc/releases/tag/v1.0
 * @see
 * https://github.com/five-embeddev/riscv-scratchpad/blob/master/baremetal-startup-cxx/src/riscv-csr.hpp
 */
namespace cpu {

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
struct FpInfo : public RegInfoBase {};

namespace csr {
/// 立即数掩码，大于这个值需要使用寄存器中转
static constexpr uint64_t kCsrImmOpMask = 0x1F;

/**
 * @brief sstatus 寄存器定义
 * @see priv-isa.pdf#10.1.1
 */
struct SstatusInfo : public RegInfoBase {
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Spie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 5;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Spp {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 8;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

/**
 * @brief stvec 寄存器定义
 * @see priv-isa.pdf#10.1.2
 */
struct StvecInfo : public RegInfoBase {
  /// 中断模式 直接
  static constexpr const uint64_t kDirect = 0x0;
  /// 中断模式 向量
  static constexpr const uint64_t kVectored = 0x1;

  struct Mode {
    using DataType = uint8_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 2;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Base {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 62;
    static constexpr uint64_t kBitMask = ~0x3;
    static constexpr uint64_t kAllSetMask = ~0x3;
  };
};

/**
 * @brief sip 寄存器定义
 * @see priv-isa.pdf#10.1.3
 */
struct SipInfo : public RegInfoBase {
  struct Ssip {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Stip {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 5;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Seip {
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
 * @brief sip 寄存器定义
 * @see priv-isa.pdf#10.1.3
 */
struct SieInfo : public RegInfoBase {
  struct Ssie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Stie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 5;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Seie {
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
 * @brief time 寄存器定义
 * @see priv-isa.pdf#10.1.4
 */
struct TimeInfo : public RegInfoBase {};

/**
 * @brief cycle 寄存器定义
 * @see priv-isa.pdf#10.1.4
 */
struct CycleInfo : public RegInfoBase {};

/**
 * @brief instret 寄存器定义
 * @see priv-isa.pdf#10.1.4
 */
struct InstretInfo : public RegInfoBase {};

/**
 * @brief sscratch 寄存器定义
 * @see priv-isa.pdf#10.1.6
 */
struct SscratchInfo : public RegInfoBase {};

/**
 * @brief sepc 寄存器定义
 * @see priv-isa.pdf#10.1.7
 */
struct SepcInfo : public RegInfoBase {};

/**
 * @brief scause 寄存器定义
 * @see priv-isa.pdf#10.1.8
 */
struct ScauseInfo : public RegInfoBase {
  enum {
    // 中断
    kInterrupt = 1ULL << 63,
    kSupervisorSoftwareInterrupt = kInterrupt + 1,
    kSupervisorTimerInterrupt = kInterrupt + 5,
    kSupervisorExternalInterrupt = kInterrupt + 9,
    kCounterOverflowInterrupt = kInterrupt + 13,

    // 异常
    kInstructionAddressMisaligned = 0,
    kInstructionAccessFault = 1,
    kIllegalInstruction = 2,
    kBreakpoint = 3,
    kLoadAddressMisaligned = 4,
    kLoadAccessFault = 5,
    kStoreAmoAddressMisaligned = 6,
    kStoreAmoAccessFault = 7,
    kEcallUserMode = 8,
    kEcallSuperMode = 9,
    kReserved4 = 10,
    kEcallMachineMode = 11,
    kInstructionPageFault = 12,
    kLoadPageFault = 13,
    kReserved5 = 14,
    kStoreAmoPageFault = 15,
    kSoftwareCheck = 18,
    kHardwareError = 19,
  };

  /// 最大中断数
  static constexpr const uint32_t kInterruptMaxCount = 16;

  /// 中断名
  static constexpr const char *const kInterruptNames[kInterruptMaxCount] = {
      "Reserved", "Supervisor Software Interrupt", "Reserved", "Reserved",
      "Reserved", "Supervisor Timer Interrupt",    "Reserved", "Reserved",
      "Reserved", "Supervisor External Interrupt", "Reserved", "Reserved",
      "Reserved", "Counter-overflow Interrupt",    "Reserved", "Reserved",
  };

  /// 最大异常数
  static constexpr const uint32_t kExceptionMaxCount = 20;

  /// 异常名
  static constexpr const char *const kExceptionNames[kExceptionMaxCount] = {
      "Instruction Address Misaligned",
      "Instruction Access Fault",
      "Illegal Instruction",
      "Breakpoint",
      "Load Address Misaligned",
      "Load Access Fault",
      "Store/AMO Address Misaligned",
      "Store/AMO Access Fault",
      "Environment Call from U-mode",
      "Environment Call from S-mode",
      "Reserved",
      "Reserved",
      "Instruction Page Fault",
      "Load Page Fault",
      "Reserved",
      "Store/AMO Page Fault",
      "Reserved",
      "Reserved",
      "SoftwareCheck",
      "HardwareError",
  };

  struct ExceptionCode {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 63;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };

  struct Interrupt {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 63;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

/**
 * @brief stval 寄存器定义
 * @see priv-isa.pdf#10.1.9
 */
struct StvalInfo : public RegInfoBase {};

/**
 * @brief satp 寄存器定义
 * @see priv-isa.pdf#10.1.11
 */
struct SatpInfo : public RegInfoBase {
  enum {
    kBare = 0,
    kSv39 = 8,
    kSv48 = 9,
    kSv57 = 10,
    kSv64 = 11,
  };

  static constexpr const char *kModeNames[] = {
      "Bare",     "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
      "Reserved", "Reserved", "SV39",     "SV48",     "SV57",     "SV64",
  };

  struct Ppn {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 44;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
  struct Asid {
    using DataType = uint16_t;
    static constexpr uint64_t kBitOffset = 44;
    static constexpr uint64_t kBitWidth = 16;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
  struct Mode {
    using DataType = uint8_t;
    static constexpr uint64_t kBitOffset = 60;
    static constexpr uint64_t kBitWidth = 4;
    static constexpr uint64_t kBitMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
    static constexpr uint64_t kAllSetMask =
        (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
  };
};

/**
 * @brief stimecmp 寄存器定义
 * @see priv-isa.pdf#16.1.1
 */
struct StimecmpInfo : public RegInfoBase {};

};  // namespace csr

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
    if constexpr (std::is_same<RegInfo, reginfo::FpInfo>::value) {
      __asm__ volatile("mv %0, fp" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrr %0, sstatus" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrr %0, stvec" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrr %0, sip" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrr %0, sie" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::TimeInfo>::value) {
      __asm__ volatile("csrr %0, time" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::CycleInfo>::value) {
      __asm__ volatile("csrr %0, cycle" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::InstretInfo>::value) {
      __asm__ volatile("csrr %0, instret" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrr %0, sscratch" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrr %0, sepc" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrr %0, scause" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrr %0, stval" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrr %0, satp" : "=r"(value) : :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StimecmpInfo>::value) {
      __asm__ volatile("csrr %0, stimecmp" : "=r"(value) : :);
    } else {
      log::Err("No Type\n");
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
    if constexpr (std::is_same<RegInfo, reginfo::FpInfo>::value) {
      __asm__ volatile("mv fp, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrw sstatus, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrw stvec, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrw sip, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrw sie, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrw sscratch, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrw sepc, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrw scause, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrw stval, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrw satp, %0" : : "r"(value) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
  }

  /**
   * 写 csr 寄存器，不通过寄存器中转
   * @param value 要写的值
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static __always_inline void WriteImm(const uint8_t value) {
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrwi sstatus, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrwi stvec, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrwi sip, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrwi sie, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrwi sscratch, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrwi sepc, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrwi scause, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrwi stval, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrwi satp, %0" : : "i"(value) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
  }

  /**
   * 通过掩码设置寄存器
   * @param mask 掩码
   */
  static __always_inline void SetBits(uint64_t mask) {
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrs zero, sstatus, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrs zero, stvec, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrs zero, sip, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrs zero, sie, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrs zero, sscratch, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrs zero, sepc, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrs zero, scause, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrs zero, stval, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrs zero, satp, %0" : : "r"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
  }

  /**
   * 清零寄存器
   * @param mask 掩码
   */
  static __always_inline void ClearBits(uint64_t mask) {
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrc zero, sstatus, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrc zero, stvec, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrc zero, sip, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrc zero, sie, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrc zero, sscratch, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrc zero, sepc, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrc zero, scause, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrc zero, stval, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrc zero, satp, %0" : : "r"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
  }

  /**
   * 通过掩码设置寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static __always_inline void SetBitsImm(const uint8_t mask) {
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrsi zero, sstatus, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrsi zero, stvec, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrsi zero, sip, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrsi zero, sie, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrsi zero, sscratch, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrsi zero, sepc, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrsi zero, scause, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrsi zero, stval, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrsi zero, satp, %0" : : "i"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
  }

  /**
   * 清零寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static __always_inline void ClearBitsImm(const uint8_t mask) {
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrci zero, sstatus, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrci zero, stvec, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrci zero, sip, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrci zero, sie, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrci zero, sscratch, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrci zero, sepc, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrci zero, scause, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrci zero, stval, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrci zero, satp, %0" : : "i"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
  }

  /**
   * 向寄存器写常数
   * @tparam value 常数的值
   */
  template <uint64_t value>
  static void WriteConst() {
    if constexpr ((value & reginfo::csr::kCsrImmOpMask) == value) {
      WriteImm(value);
    } else {
      Write(value);
    }
  }

  /**
   * 通过掩码写寄存器
   * @tparam mask 掩码
   */
  template <uint64_t mask>
  static void SetConst() {
    if constexpr ((mask & reginfo::csr::kCsrImmOpMask) == mask) {
      SetBitsImm(mask);
    } else {
      SetBits(mask);
    }
  }

  /**
   * 通过掩码清零寄存器
   * @tparam mask 掩码
   */
  template <uint64_t mask>
  static void ClearConst() {
    if constexpr ((mask & reginfo::csr::kCsrImmOpMask) == mask) {
      ClearBitsImm(mask);
    } else {
      ClearBits(mask);
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
    typename RegInfo::DataType old_value{};
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrw %0, sstatus, %1"
                       : "=r"(old_value)
                       : "r"(value)
                       :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrw %0, stvec, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrw %0, sip, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrw %0, sie, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrw %0, sscratch, %1"
                       : "=r"(old_value)
                       : "r"(value)
                       :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrw %0, sepc, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrw %0, scause, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrw %0, stval, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrw %0, satp, %1" : "=r"(old_value) : "r"(value) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
    return old_value;
  }

  /**
   * 先读后写寄存器，不通过寄存器中转
   * @param value 要写的值
   * @return RegInfo::DataType 寄存器的值
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static __always_inline RegInfo::DataType ReadWriteImm(const uint8_t value) {
    typename RegInfo::DataType old_value{};
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrwi %0, sstatus, %1"
                       : "=r"(old_value)
                       : "i"(value)
                       :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrwi %0, stvec, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrwi %0, sip, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrwi %0, sie, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrwi %0, sscratch, %1"
                       : "=r"(old_value)
                       : "i"(value)
                       :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrwi %0, sepc, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrwi %0, scause, %1"
                       : "=r"(old_value)
                       : "i"(value)
                       :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrwi %0, stval, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrwi %0, satp, %1" : "=r"(old_value) : "i"(value) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
    return old_value;
  }

  /**
   * 先读后写常数到寄存器
   * @tparam value 要写的值
   * @return RegInfo::DataType 寄存器的值
   */
  template <uint64_t value>
  static RegInfo::DataType ReadWriteConst() {
    if constexpr ((value & reginfo::csr::kCsrImmOpMask) == value) {
      return ReadWriteRegBase<RegInfo>::ReadWriteImm(value);
    } else {
      return ReadWrite(value);
    }
  }

  /**
   * 先读后通过掩码设置寄存器
   * @param mask 掩码
   * @return RegInfo::DataType 寄存器的值
   */
  static __always_inline RegInfo::DataType ReadSetBits(uint64_t mask) {
    typename RegInfo::DataType value{};
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrs %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrs %0, stvec, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrs %0, sip, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrs %0, sie, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrs %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrs %0, sepc, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrs %0, scause, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrs %0, stval, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrs %0, satp, %1" : "=r"(value) : "r"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * 先读后通过掩码设置寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static __always_inline RegInfo::DataType ReadSetBitsImm(const uint8_t mask) {
    typename RegInfo::DataType value{};
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrsi %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrsi %0, stvec, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrsi %0, sip, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrsi %0, sie, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrsi %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrsi %0, sepc, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrsi %0, scause, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrsi %0, stval, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrsi %0, satp, %1" : "=r"(value) : "i"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * 通过常数掩码先读后写寄存器
   * @tparam mask 掩码
   * @return RegInfo::DataType 寄存器的值
   */
  template <uint64_t mask>
  static RegInfo::DataType ReadSetBitsConst() {
    if constexpr ((mask & reginfo::csr::kCsrImmOpMask) == mask) {
      return ReadSetBitsImm(mask);
    } else {
      return ReadSetBits(mask);
    }
  }

  /**
   * 先读后清零寄存器
   * @param mask 掩码
   * @return RegInfo::DataType 寄存器的值
   */
  static __always_inline RegInfo::DataType ReadClearBits(uint64_t mask) {
    typename RegInfo::DataType value{};
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrc %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrc %0, stvec, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrc %0, sip, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrc %0, sie, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrc %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrc %0, sepc, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrc %0, scause, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrc %0, stval, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrc %0, satp, %1" : "=r"(value) : "r"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * 先读后清零寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static __always_inline RegInfo::DataType ReadClearBitsImm(
      const uint8_t mask) {
    typename RegInfo::DataType value{};
    if constexpr (std::is_same<RegInfo, reginfo::csr::SstatusInfo>::value) {
      __asm__ volatile("csrrci %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvecInfo>::value) {
      __asm__ volatile("csrrci %0, stvec, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SipInfo>::value) {
      __asm__ volatile("csrrci %0, sip, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SieInfo>::value) {
      __asm__ volatile("csrrci %0, sie, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::SscratchInfo>::value) {
      __asm__ volatile("csrrci %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SepcInfo>::value) {
      __asm__ volatile("csrrci %0, sepc, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::ScauseInfo>::value) {
      __asm__ volatile("csrrci %0, scause, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo,
                                      reginfo::csr::StvalInfo>::value) {
      __asm__ volatile("csrrci %0, stval, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<RegInfo, reginfo::csr::SatpInfo>::value) {
      __asm__ volatile("csrrci %0, satp, %1" : "=r"(value) : "i"(mask) :);
    } else {
      log::Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * 通过常数掩码先读后清零寄存器
   * @tparam mask 掩码
   * @return RegInfo::DataType 寄存器的值
   */
  template <uint64_t mask>
  static RegInfo::DataType ReadClearBitsConst() {
    if constexpr ((mask & reginfo::csr::kCsrImmOpMask) == mask) {
      return WriteOnlyRegBase<RegInfo>::ReadClearBitsImm(mask);
    } else {
      return ReadClearBits(mask);
    }
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
    return (typename RegInfo::DataType)((Reg::Read() & RegInfo::kBitMask) >>
                                        RegInfo::kBitOffset);
  }

  /**
   * 从指定的值获取对应 Reg 的由 RegInfo 规定的指定位的值
   * @param value 指定的值
   * @return RegInfo::DataType 指定位值的信息
   */
  static __always_inline RegInfo::DataType Get(RegInfo::DataType value) {
    return (typename RegInfo::DataType)((value & RegInfo::kBitMask) >>
                                        RegInfo::kBitOffset);
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
  static __always_inline void Set() {
    if constexpr ((RegInfo::kBitMask & reginfo::csr::kCsrImmOpMask) ==
                  RegInfo::kBitMask) {
      Reg::SetBitsImm(RegInfo::kBitMask);
    } else {
      Reg::SetBits(RegInfo::kBitMask);
    }
  }

  /**
   * 清零对应 Reg 的由 RegInfo 规定的指定位
   */
  static __always_inline void Clear() {
    if constexpr ((RegInfo::kBitMask & reginfo::csr::kCsrImmOpMask) ==
                  RegInfo::kBitMask) {
      Reg::ClearBitsImm(RegInfo::kBitMask);
    } else {
      Reg::ClearBits(RegInfo::kBitMask);
    }
  }
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
class Fp : public ReadWriteRegBase<reginfo::FpInfo> {
 public:
  friend sk_std::ostream &operator<<(sk_std::ostream &os, const Fp &fp) {
    printf("val: 0x%p", (void *)fp.Read());
    return os;
  }
};

/// 通用寄存器
struct AllXreg {
  Fp fp;
};

namespace csr {

class Sstatus : public ReadWriteRegBase<reginfo::csr::SstatusInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SstatusInfo>,
                 reginfo::csr::SstatusInfo::Sie>
      sie;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SstatusInfo>,
                 reginfo::csr::SstatusInfo::Spie>
      spie;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SstatusInfo>,
                 reginfo::csr::SstatusInfo::Spp>
      spp;

  /// @name 构造/析构函数
  /// @{
  Sstatus() = default;
  Sstatus(const Sstatus &) = delete;
  Sstatus(Sstatus &&) = delete;
  auto operator=(const Sstatus &) -> Sstatus & = delete;
  auto operator=(Sstatus &&) -> Sstatus & = delete;
  virtual ~Sstatus() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Sstatus &sstatus) {
    auto sie = sstatus.sie.Get();
    auto spie = sstatus.spie.Get();
    auto spp = sstatus.spp.Get();
    printf("val: 0x%p, sie: %s, spie: %s, spp: %s", (void *)sstatus.Read(),
           (sie == true ? "Enable" : "Disable"),
           (spie == true ? "Enable" : "Disable"),
           (spp == true ? "S Mode" : "U Mode")

    );
    return os;
  }
};

class Stvec : public ReadWriteRegBase<reginfo::csr::StvecInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::csr::StvecInfo>,
                 reginfo::csr::StvecInfo::Base>
      base;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::StvecInfo>,
                 reginfo::csr::StvecInfo::Mode>
      mode;

  void SetDirect(uint64_t addr) {
    base.Write(addr);
    mode.Write(reginfo::csr::StvecInfo::kDirect);
  }

  /// @name 构造/析构函数
  /// @{
  Stvec() = default;
  Stvec(const Stvec &) = delete;
  Stvec(Stvec &&) = delete;
  auto operator=(const Stvec &) -> Stvec & = delete;
  auto operator=(Stvec &&) -> Stvec & = delete;
  virtual ~Stvec() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Stvec &stvec) {
    auto mode = stvec.mode.Get();
    auto base = stvec.base.Get();
    printf("val: 0x%p, mode: %s, base: 0x%lX", (void *)stvec.Read(),
           (mode == reginfo::csr::StvecInfo::kDirect ? "Direct" : "Vectored"),
           base);
    return os;
  }
};

class Sip : public ReadWriteRegBase<reginfo::csr::SipInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SipInfo>,
                 reginfo::csr::SipInfo::Ssip>
      ssip;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SipInfo>,
                 reginfo::csr::SipInfo::Stip>
      stip;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SipInfo>,
                 reginfo::csr::SipInfo::Seip>
      seip;

  /// @name 构造/析构函数
  /// @{
  Sip() = default;
  Sip(const Sip &) = delete;
  Sip(Sip &&) = delete;
  auto operator=(const Sip &) -> Sip & = delete;
  auto operator=(Sip &&) -> Sip & = delete;
  virtual ~Sip() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Sip &sip) {
    auto ssip = sip.ssip.Get();
    auto stip = sip.stip.Get();
    auto seip = sip.seip.Get();
    printf("val: 0x%p, ssie: %s, stie: %s, seie: %s", (void *)sip.Read(),
           (ssip == true ? "Enable" : "Disable"),
           (stip == true ? "Enable" : "Disable"),
           (seip == true ? "Enable" : "Disable"));
    return os;
  }
};

class Sie : public ReadWriteRegBase<reginfo::csr::SieInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SieInfo>,
                 reginfo::csr::SieInfo::Ssie>
      ssie;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SieInfo>,
                 reginfo::csr::SieInfo::Stie>
      stie;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SieInfo>,
                 reginfo::csr::SieInfo::Seie>
      seie;

  /// @name 构造/析构函数
  /// @{
  Sie() = default;
  Sie(const Sie &) = delete;
  Sie(Sie &&) = delete;
  auto operator=(const Sie &) -> Sie & = delete;
  auto operator=(Sie &&) -> Sie & = delete;
  virtual ~Sie() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Sie &sie) {
    auto ssie = sie.ssie.Get();
    auto stie = sie.stie.Get();
    auto seie = sie.seie.Get();
    printf("val: 0x%p, ssie: %s, stie: %s, seie: %s", (void *)sie.Read(),
           (ssie == true ? "Enable" : "Disable"),
           (stie == true ? "Enable" : "Disable"),
           (seie == true ? "Enable" : "Disable"));
    return os;
  }
};

class Time : public ReadOnlyRegBase<reginfo::csr::TimeInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Time() = default;
  Time(const Time &) = delete;
  Time(Time &&) = delete;
  auto operator=(const Time &) -> Time & = delete;
  auto operator=(Time &&) -> Time & = delete;
  virtual ~Time() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Time &time) {
    printf("val: 0x%p", (void *)time.Read());
    return os;
  }
};

class Cycle : public ReadOnlyRegBase<reginfo::csr::CycleInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Cycle() = default;
  Cycle(const Cycle &) = delete;
  Cycle(Cycle &&) = delete;
  auto operator=(const Cycle &) -> Cycle & = delete;
  auto operator=(Cycle &&) -> Cycle & = delete;
  virtual ~Cycle() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Cycle &cycle) {
    printf("val: 0x%p", (void *)cycle.Read());
    return os;
  }
};

class Instret : public ReadOnlyRegBase<reginfo::csr::InstretInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Instret() = default;
  Instret(const Instret &) = delete;
  Instret(Instret &&) = delete;
  auto operator=(const Instret &) -> Instret & = delete;
  auto operator=(Instret &&) -> Instret & = delete;
  virtual ~Instret() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Instret &instret) {
    printf("val: 0x%p", (void *)instret.Read());
    return os;
  }
};

class Sscratch : public ReadWriteRegBase<reginfo::csr::SscratchInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Sscratch() = default;
  Sscratch(const Sscratch &) = delete;
  Sscratch(Sscratch &&) = delete;
  auto operator=(const Sscratch &) -> Sscratch & = delete;
  auto operator=(Sscratch &&) -> Sscratch & = delete;
  virtual ~Sscratch() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Sscratch &sscratch) {
    printf("val: 0x%p", (void *)sscratch.Read());
    return os;
  }
};

class Sepc : public ReadWriteRegBase<reginfo::csr::SepcInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Sepc() = default;
  Sepc(const Sepc &) = delete;
  Sepc(Sepc &&) = delete;
  auto operator=(const Sepc &) -> Sepc & = delete;
  auto operator=(Sepc &&) -> Sepc & = delete;
  virtual ~Sepc() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Sepc &sepc) {
    printf("val: 0x%p", (void *)sepc.Read());
    return os;
  }
};

class Scause : public ReadWriteRegBase<reginfo::csr::ScauseInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::csr::ScauseInfo>,
                 reginfo::csr::ScauseInfo::Interrupt>
      interrupt;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::ScauseInfo>,
                 reginfo::csr::ScauseInfo::ExceptionCode>
      exception_code;

  /// @name 构造/析构函数
  /// @{
  Scause() = default;
  Scause(const Scause &) = delete;
  Scause(Scause &&) = delete;
  auto operator=(const Scause &) -> Scause & = delete;
  auto operator=(Scause &&) -> Scause & = delete;
  virtual ~Scause() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Scause &scause) {
    auto exception_code = scause.exception_code.Get();
    auto interrupt = scause.interrupt.Get();
    printf("val: 0x%p, exception_code: 0x%lX, interrupt: %s, name: %s",
           (void *)scause.Read(), exception_code, interrupt ? "Yes" : "No",
           interrupt
               ? reginfo::csr::ScauseInfo::kInterruptNames[exception_code]
               : reginfo::csr::ScauseInfo::kExceptionNames[exception_code]);
    return os;
  }
};

class Stval : public ReadWriteRegBase<reginfo::csr::StvalInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Stval() = default;
  Stval(const Stval &) = delete;
  Stval(Stval &&) = delete;
  auto operator=(const Stval &) -> Stval & = delete;
  auto operator=(Stval &&) -> Stval & = delete;
  virtual ~Stval() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Stval &stval) {
    printf("val: 0x%p", (void *)stval.Read());
    return os;
  }
};

class Satp : public ReadWriteRegBase<reginfo::csr::SatpInfo> {
 public:
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SatpInfo>,
                 reginfo::csr::SatpInfo::Ppn>
      ppn;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SatpInfo>,
                 reginfo::csr::SatpInfo::Asid>
      asid;
  ReadWriteField<ReadWriteRegBase<reginfo::csr::SatpInfo>,
                 reginfo::csr::SatpInfo::Mode>
      mode;

  /// @name 构造/析构函数
  /// @{
  Satp() = default;
  Satp(const Satp &) = delete;
  Satp(Satp &&) = delete;
  auto operator=(const Satp &) -> Satp & = delete;
  auto operator=(Satp &&) -> Satp & = delete;
  virtual ~Satp() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Satp &satp) {
    auto ppn = satp.ppn.Get();
    auto asid = satp.asid.Get();
    auto mode = satp.mode.Get();
    printf("val: 0x%p, ppn: 0x%lX, asid: 0x%X, mode: %s", (void *)satp.Read(),
           ppn, asid, reginfo::csr::SatpInfo::kModeNames[mode]);
    return os;
  }
};

class Stimecmp : public ReadOnlyRegBase<reginfo::csr::StimecmpInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Stimecmp() = default;
  Stimecmp(const Stimecmp &) = delete;
  Stimecmp(Stimecmp &&) = delete;
  auto operator=(const Stimecmp &) -> Stimecmp & = delete;
  auto operator=(Stimecmp &&) -> Stimecmp & = delete;
  virtual ~Stimecmp() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Stimecmp &stimecmp) {
    printf("val: 0x%p", (void *)stimecmp.Read());
    return os;
  }
};

class AllCsr {
 public:
  csr::Sstatus sstatus;
  csr::Stvec stvec;
  csr::Sip sip;
  csr::Sie sie;
  csr::Time time;
  csr::Cycle cycle;
  csr::Instret instret;
  csr::Sscratch sscratch;
  csr::Sepc sepc;
  csr::Scause scause;
  csr::Stval stval;
  csr::Satp satp;
  csr::Stimecmp stimecmp;

  /// @name 构造/析构函数
  /// @{
  AllCsr() = default;
  AllCsr(const AllCsr &) = delete;
  AllCsr(AllCsr &&) = delete;
  auto operator=(const AllCsr &) -> AllCsr & = delete;
  auto operator=(AllCsr &&) -> AllCsr & = delete;
  virtual ~AllCsr() = default;
  /// @}
};

};  // namespace csr

};  // namespace

// 第四部分：访问接口
[[maybe_unused]] static AllXreg kAllXreg;
[[maybe_unused]] static csr::AllCsr kAllCsr;

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_

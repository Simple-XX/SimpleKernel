
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "iostream"
#include "stdio.h"

/**
 * riscv64 cpu 相关定义
 * @see priv-isa-asciidoc_20240411.pdf
 * https://github.com/riscv/riscv-isa-manual/releases/tag/20240411/priv-isa-asciidoc.pdf
 * @see riscv-abi.pdf
 * https://github.com/riscv-non-isa/riscv-elf-psabi-doc/releases/tag/v1.0
 * @see
 * https://github.com/five-embeddev/riscv-scratchpad/blob/master/baremetal-startup-cxx/src/riscv-csr.hpp
 */
class Cpu {
 public:
  /**
   * Privilege Levels
   * @see priv-isa-asciidoc_20240411.pdf#1.2
   */
  enum {
    kPrivilegeLevelUser = 0,
    kPrivilegeLevelSuper = 1,
    kPrivilegeLevelMachine = 3,
  };

  /**
   * @brief Xstatus 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.6
   */
  class Xstatus {
   public:
    /// User Interrupt Enable
    static constexpr const uint64_t kXstatusUie = 1 << 0;
    /// Supervisor Interrupt Enable
    static constexpr const uint64_t kXstatusSie = 1 << 1;
    /// User Previous Interrupt Enable
    static constexpr const uint64_t kXstatusUpie = 1 << 4;
    /// Supervisor Previous Interrupt Enable
    static constexpr const uint64_t kXstatusSpie = 1 << 5;
    /// Previous mode, 1=Supervisor, 0=User
    static constexpr const uint64_t kXstatusSpp = 1 << 8;

    union {
      struct {
        // Reserved Writes Preserve Values, Reads Ignore Values 1
        uint64_t wpri1 : 1;
        // The SIE bit enables or disables all interrupts in supervisor mode.
        uint64_t sie : 1;
        uint64_t wpri2 : 1;
        // The MIE bit enables or disables all interrupts in machine mode.
        uint64_t mie : 1;
        uint64_t wpri3 : 1;
        // The SPIE bit indicates whether supervisor interrupts were enabled
        // prior to trapping into supervisor mode.
        uint64_t spie : 1;
        // The MBE, SBE, and UBE bits in mstatus and mstatush are WARL fields
        // that control the endianness of memory accesses other than instruction
        // fetches. Instruction fetches are always little-endian.
        uint64_t ube : 1;
        uint64_t mpie : 1;
        // The SPP bit indicates the privilege level at which a hart was
        // executing before entering supervisor mode.
        // xPP holds the previous privilege mode.
        uint64_t spp : 1;
        uint64_t vs : 2;
        uint64_t mpp : 2;
        // The FS field encodes the status of the floating-point unit, including
        // the CSR fcsr and floating-point data registers f0–f31.
        uint64_t fs : 2;
        // The XS field encodes the status of additional user-mode extensions
        // and associated state.
        uint64_t xs : 2;
        // Modify PRiVilege
        uint64_t mprv : 1;
        // permit Supervisor User Memory access
        uint64_t sum : 1;
        // Make eXecutable Readable
        uint64_t mxr : 1;
        // Trap Virtual Memory
        uint64_t tvm : 1;
        // Timeout Wait
        uint64_t tw : 1;
        // Trap SRET
        uint64_t tsr : 1;
        uint64_t wpri4 : 9;
        // U-mode XLEN
        uint64_t uxl : 2;
        // S-mode XLEN
        uint64_t sxl : 2;
        uint64_t sbe : 1;
        uint64_t mbe : 1;
        uint64_t wpri5 : 25;
        // The SD bit is Read-only and is set when either the FS or XS bits
        // encode a Dirty state (i.e., SD=((FS==11) OR (XS==11))).
        uint64_t sd : 1;
      } xstatus_;
      uint64_t val_;
    };

    explicit Xstatus(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xstatus() = default;
    Xstatus(const Xstatus &) = default;
    Xstatus(Xstatus &&) = default;
    auto operator=(const Xstatus &) -> Xstatus & = default;
    auto operator=(Xstatus &&) -> Xstatus & = default;
    ~Xstatus() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xstatus &xstatus) {
      printf("val: 0x%p, sie: %s, spie: %s, spp: %s", xstatus.val_,
             (xstatus.xstatus_.sie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.spie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.spp == true ? "S Mode" : "U Mode")

      );
      return os;
    }
  };

  /**
   * @brief Xtvec 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.7
   */
  class Xtvec {
   public:
    /// 中断模式 直接
    static constexpr const uint64_t kDirect = 0x0;
    /// 中断模式 向量
    static constexpr const uint64_t kVectored = 0x1;

    union {
      struct {
        uint64_t mode : 2;
        uint64_t base : 62;
      } xtvec_;
      uint64_t val_;
    };

    explicit Xtvec(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xtvec() = default;
    Xtvec(const Xtvec &) = default;
    Xtvec(Xtvec &&) = default;
    auto operator=(const Xtvec &) -> Xtvec & = default;
    auto operator=(Xtvec &&) -> Xtvec & = default;
    ~Xtvec() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xtvec &xtvec) {
      printf("val: 0x%p, mode: %s, base: 0x%p", xtvec.val_,
             (xtvec.xtvec_.mode == kDirect ? "Direct" : "Vectored"),
             xtvec.xtvec_.base);
      return os;
    }
  };

  /**
   * @brief Xip 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.9
   */
  class Xip {
   public:
    // Supervisor Interrupt Pending
    /// software
    static constexpr const uint64_t kSsip = 1 << 1;
    /// timer
    static constexpr const uint64_t kStip = 1 << 5;
    /// external
    static constexpr const uint64_t kSeip = 1 << 9;

    union {
      struct {
        uint64_t zero0 : 1;
        /// Supervisor Software Interrupt Pending
        uint64_t ssip : 1;
        uint64_t zero2 : 1;
        uint64_t msip : 1;
        uint64_t zero4 : 1;
        /// Supervisor Timer Interrupt Pending
        uint64_t stip : 1;
        uint64_t zero6 : 1;
        /// Machine Timer Interrupt Pending
        uint64_t mtip : 1;
        /// User External Interrupt Pending
        uint64_t zero8 : 1;
        /// Supervisor External Interrupt Pending
        uint64_t seip : 1;
        uint64_t zero10 : 1;
        /// Machine External Interrupt Pending
        uint64_t meip : 1;
        uint64_t zero12 : 1;
        uint64_t lcofip : 1;
        uint64_t zero14_15 : 1;
        uint64_t warl : 48;
      } xip_;
      uint64_t val_;
    };

    explicit Xip(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xip() = default;
    Xip(const Xip &) = default;
    Xip(Xip &&) = default;
    auto operator=(const Xip &) -> Xip & = default;
    auto operator=(Xip &&) -> Xip & = default;
    ~Xip() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xip &xip) {
      printf("val: 0x%p, ssie: %s, stie: %s, seie: %s", xip.val_,
             (xip.xip_.ssip == true ? "Enable" : "Disable"),
             (xip.xip_.stip == true ? "Enable" : "Disable"),
             (xip.xip_.seip == true ? "Enable" : "Disable"));
      return os;
    }
  };

  /**
   * @brief Xie 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.9
   */
  class Xie {
   public:
    // Supervisor Interrupt Enable
    /// software
    static constexpr const uint64_t kSsie = 1 << 1;
    /// timer
    static constexpr const uint64_t kStie = 1 << 5;
    /// external
    static constexpr const uint64_t kSeie = 1 << 9;

    union {
      struct {
        uint64_t zero0 : 1;
        /// Supervisor Software Interrupt Enable
        uint64_t ssie : 1;
        uint64_t zero2 : 1;
        uint64_t msie : 1;
        uint64_t zero4 : 1;
        /// Supervisor Timer Interrupt Enable
        uint64_t stie : 1;
        uint64_t zero6 : 1;
        /// Machine Timer Interrupt Enable
        uint64_t mtie : 1;
        uint64_t zero8 : 1;
        /// Supervisor External Interrupt Enable
        uint64_t seie : 1;
        uint64_t zero10 : 1;
        /// Machine External Interrupt Enable
        uint64_t meie : 1;
        uint64_t zero12 : 1;
        uint64_t lcofie : 1;
        uint64_t zero14_15 : 1;
        /// warl
        uint64_t wpri4 : 48;
      } xie_;
      uint64_t val_;
    };

    explicit Xie(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xie() = default;
    Xie(const Xie &) = default;
    Xie(Xie &&) = default;
    auto operator=(const Xie &) -> Xie & = default;
    auto operator=(Xie &&) -> Xie & = default;
    ~Xie() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xie &xie) {
      printf("val: 0x%p, ssie: %s, stie: %s, seie: %s", xie.val_,
             (xie.xie_.ssie == true ? "Enable" : "Disable"),
             (xie.xie_.stie == true ? "Enable" : "Disable"),
             (xie.xie_.seie == true ? "Enable" : "Disable"));
      return os;
    }
  };

  /**
   * @brief Xcause 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#3.1.15
   */
  class Xcause {
   public:
    enum {
      // 中断
      kInterrupt = 1ULL << 63,
      kUserSoftwareInterrupt = kInterrupt + 0,
      kSupervisorSoftwareInterrupt = kInterrupt + 1,
      kReserved1 = kInterrupt + 2,
      kMachineSoftwareInterrupt = kInterrupt + 3,
      kUserTimerInterrupt = kInterrupt + 4,
      kSupervisorTimerInterrupt = kInterrupt + 5,
      kReserved2 = kInterrupt + 6,
      kMachineTimerInterrupt = kInterrupt + 7,
      kUserExternalInterrupt = kInterrupt + 8,
      kSupervisorExternalInterrupt = kInterrupt + 9,
      kReserved3 = kInterrupt + 10,
      kMachineExternalInterrupt = kInterrupt + 11,

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
    };

    /// 最大中断数
    static constexpr const uint32_t kInterruptMaxCount = 12;

    /// 中断名
    static constexpr const char *const kInterruptNames[kInterruptMaxCount] = {
        "User Software Interrupt",
        "Supervisor Software Interrupt",
        "Reserved",
        "Machine Software Interrupt",
        "User Timer Interrupt",
        "Supervisor Timer Interrupt",
        "Reserved",
        "Machine Timer Interrupt",
        "User External Interrupt",
        "Supervisor External Interrupt",
        "Reserved",
        "Machine External Interrupt",
    };

    /// 最大异常数
    static constexpr const uint32_t kExceptionMaxCount = 16;

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
        "Environment Call from M-mode",
        "Instruction Page Fault",
        "Load Page Fault",
        "Reserved",
        "Store/AMO Page Fault",
    };

    union {
      struct {
        uint64_t exception_code : 63;
        uint64_t interrupt : 1;
      } xcause_;
      uint64_t val_;
    };

    explicit Xcause(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Xcause() = default;
    Xcause(const Xcause &) = default;
    Xcause(Xcause &&) = default;
    auto operator=(const Xcause &) -> Xcause & = default;
    auto operator=(Xcause &&) -> Xcause & = default;
    ~Xcause() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Xcause &xcause) {
      printf("val: 0x%p, exception_code: 0x%p, interrupt: %s, name: %s",
             xcause.val_, xcause.xcause_.exception_code,
             xcause.xcause_.interrupt ? "Yes" : "No",
             xcause.xcause_.interrupt
                 ? kInterruptNames[xcause.xcause_.exception_code]
                 : kExceptionNames[xcause.xcause_.exception_code]);
      return os;
    }
  };

  /**
   * @brief satp 寄存器定义
   * @see priv-isa-asciidoc_20240411.pdf#10.1.11
   */
  class Satp {
   public:
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

    union {
      struct {
        uint64_t ppn : 44;
        uint64_t asid : 16;
        uint64_t mode : 4;
      } satp_;
      uint64_t val_;
    };

    explicit Satp(uint64_t val) : val_(val) {}

    /// @name 构造/析构函数
    /// @{
    Satp() = default;
    Satp(const Satp &) = default;
    Satp(Satp &&) = default;
    auto operator=(const Satp &) -> Satp & = default;
    auto operator=(Satp &&) -> Satp & = default;
    ~Satp() = default;
    /// @}

    friend std::ostream &operator<<(std::ostream &os, const Satp &satp) {
      printf("val: 0x%p, ppn: 0x%p, asid: 0x%p, mode: %s", satp.val_,
             satp.satp_.ppn, satp.satp_.asid, kModeNames[satp.satp_.mode]);
      return os;
    }
  };

  /**
   * @brief 通用寄存器
   */
  struct Xregs {
    uint64_t zero;
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    friend std::ostream &operator<<(std::ostream &os, const Xregs &xregs) {
      printf("zero: 0x%p, ", xregs.zero);
      printf("ra: 0x%p, ", xregs.ra);
      printf("sp: 0x%p, ", xregs.sp);
      printf("gp: 0x%p\n", xregs.gp);
      printf("tp: 0x%p, ", xregs.tp);
      printf("t0: 0x%p, ", xregs.t0);
      printf("t1: 0x%p, ", xregs.t1);
      printf("t2: 0x%p\n", xregs.t2);
      printf("s0: 0x%p, ", xregs.s0);
      printf("s1: 0x%p, ", xregs.s1);
      printf("a0: 0x%p, ", xregs.a0);
      printf("a1: 0x%p\n", xregs.a1);
      printf("a2: 0x%p, ", xregs.a2);
      printf("a3: 0x%p, ", xregs.a3);
      printf("a4: 0x%p, ", xregs.a4);
      printf("a5: 0x%p\n", xregs.a5);
      printf("a6: 0x%p, ", xregs.a6);
      printf("a7: 0x%p, ", xregs.a7);
      printf("s2: 0x%p, ", xregs.s2);
      printf("s3: 0x%p\n", xregs.s3);
      printf("s4: 0x%p, ", xregs.s4);
      printf("s5: 0x%p, ", xregs.s5);
      printf("s6: 0x%p, ", xregs.s6);
      printf("s7: 0x%p\n", xregs.s7);
      printf("s8: 0x%p, ", xregs.s8);
      printf("s9: 0x%p, ", xregs.s9);
      printf("s10: 0x%p, ", xregs.s10);
      printf("s11: 0x%p\n", xregs.s11);
      printf("t3: 0x%p, ", xregs.t3);
      printf("t4: 0x%p, ", xregs.t4);
      printf("t5: 0x%p, ", xregs.t5);
      printf("t6: 0x%p", xregs.t6);
      return os;
    }
  };

  /**
   * @brief 浮点寄存器
   */
  struct Fregs {
    uintptr_t ft0;
    uintptr_t ft1;
    uintptr_t ft2;
    uintptr_t ft3;
    uintptr_t ft4;
    uintptr_t ft5;
    uintptr_t ft6;
    uintptr_t ft7;
    uintptr_t fs0;
    uintptr_t fs1;
    uintptr_t fa0;
    uintptr_t fa1;
    uintptr_t fa2;
    uintptr_t fa3;
    uintptr_t fa4;
    uintptr_t fa5;
    uintptr_t fa6;
    uintptr_t fa7;
    uintptr_t fs2;
    uintptr_t fs3;
    uintptr_t fs4;
    uintptr_t fs5;
    uintptr_t fs6;
    uintptr_t fs7;
    uintptr_t fs8;
    uintptr_t fs9;
    uintptr_t fs10;
    uintptr_t fs11;
    uintptr_t ft8;
    uintptr_t ft9;
    uintptr_t ft10;
    uintptr_t ft11;
    friend std::ostream &operator<<(std::ostream &os, const Fregs &fregs) {
      printf("ft0: 0x%p, ", fregs.ft0);
      printf("ft1: 0x%p, ", fregs.ft1);
      printf("ft2: 0x%p, ", fregs.ft2);
      printf("ft3: 0x%p\n", fregs.ft3);
      printf("ft4: 0x%p, ", fregs.ft4);
      printf("ft5: 0x%p, ", fregs.ft5);
      printf("ft6: 0x%p, ", fregs.ft6);
      printf("ft7: 0x%p\n", fregs.ft7);
      printf("fs0: 0x%p, ", fregs.fs0);
      printf("fs1: 0x%p, ", fregs.fs1);
      printf("fa0: 0x%p, ", fregs.fa0);
      printf("fa1: 0x%p\n", fregs.fa1);
      printf("fa2: 0x%p, ", fregs.fa2);
      printf("fa3: 0x%p, ", fregs.fa3);
      printf("fa4: 0x%p, ", fregs.fa4);
      printf("fa5: 0x%p\n", fregs.fa5);
      printf("fa6: 0x%p, ", fregs.fa6);
      printf("fa7: 0x%p, ", fregs.fa7);
      printf("fs2: 0x%p, ", fregs.fs2);
      printf("fs3: 0x%p\n", fregs.fs3);
      printf("fs4: 0x%p, ", fregs.fs4);
      printf("fs5: 0x%p, ", fregs.fs5);
      printf("fs6: 0x%p, ", fregs.fs6);
      printf("fs7: 0x%p\n", fregs.fs7);
      printf("fs8: 0x%p, ", fregs.fs8);
      printf("fs9: 0x%p, ", fregs.fs9);
      printf("fs10: 0x%p, ", fregs.fs10);
      printf("fs11: 0x%p\n", fregs.fs11);
      printf("ft8: 0x%p, ", fregs.ft8);
      printf("ft9: 0x%p, ", fregs.ft9);
      printf("ft10: 0x%p, ", fregs.ft10);
      printf("ft11: 0x%p", fregs.ft11);
      return os;
    }
  };

  /**
   * @brief 调用者保存寄存器
   * @see riscv-abi.pdf#1.1
   * @see riscv-abi.pdf#1.2
   */
  struct CallerRegs {
    uintptr_t ra;
    uintptr_t t0;
    uintptr_t t1;
    uintptr_t t2;
    uintptr_t a0;
    uintptr_t a1;
    uintptr_t a2;
    uintptr_t a3;
    uintptr_t a4;
    uintptr_t a5;
    uintptr_t a6;
    uintptr_t a7;
    uintptr_t t3;
    uintptr_t t4;
    uintptr_t t5;
    uintptr_t t6;
    uintptr_t ft0;
    uintptr_t ft1;
    uintptr_t ft2;
    uintptr_t ft3;
    uintptr_t ft4;
    uintptr_t ft5;
    uintptr_t ft6;
    uintptr_t ft7;
    uintptr_t fa0;
    uintptr_t fa1;
    uintptr_t fa2;
    uintptr_t fa3;
    uintptr_t fa4;
    uintptr_t fa5;
    uintptr_t fa6;
    uintptr_t fa7;
    uintptr_t ft8;
    uintptr_t ft9;
    uintptr_t ft10;
    uintptr_t ft11;
    friend std::ostream &operator<<(std::ostream &os,
                                    const CallerRegs &caller_regs) {
      printf("ra: 0x%p, ", caller_regs.ra);
      printf("t0: 0x%p, ", caller_regs.t0);
      printf("t1: 0x%p, ", caller_regs.t1);
      printf("t2: 0x%p, ", caller_regs.t2);
      printf("a0: 0x%p\n", caller_regs.a0);
      printf("a1: 0x%p, ", caller_regs.a1);
      printf("a2: 0x%p, ", caller_regs.a2);
      printf("a3: 0x%p, ", caller_regs.a3);
      printf("a4: 0x%p\n", caller_regs.a4);
      printf("a5: 0x%p, ", caller_regs.a5);
      printf("a6: 0x%p, ", caller_regs.a6);
      printf("a7: 0x%p, ", caller_regs.a7);
      printf("t3: 0x%p\n", caller_regs.t3);
      printf("t4: 0x%p, ", caller_regs.t4);
      printf("t5: 0x%p, ", caller_regs.t5);
      printf("t6: 0x%p, ", caller_regs.t6);
      printf("ft0: 0x%p\n", caller_regs.ft0);
      printf("ft1: 0x%p, ", caller_regs.ft1);
      printf("ft2: 0x%p, ", caller_regs.ft2);
      printf("ft3: 0x%p, ", caller_regs.ft3);
      printf("ft4: 0x%p\n", caller_regs.ft4);
      printf("ft5: 0x%p, ", caller_regs.ft5);
      printf("ft6: 0x%p, ", caller_regs.ft6);
      printf("ft7: 0x%p, ", caller_regs.ft7);
      printf("fa0: 0x%p\n", caller_regs.fa0);
      printf("fa1: 0x%p", caller_regs.fa1);
      printf("fa2: 0x%p", caller_regs.fa2);
      printf("fa3: 0x%p", caller_regs.fa3);
      printf("fa4: 0x%p", caller_regs.fa4);
      printf("fa5: 0x%p", caller_regs.fa5);
      printf("fa6: 0x%p", caller_regs.fa6);
      printf("fa7: 0x%p", caller_regs.fa7);
      printf("ft8: 0x%p", caller_regs.ft8);
      printf("ft9: 0x%p", caller_regs.ft9);
      printf("ft10: 0x%p", caller_regs.ft10);
      printf("ft11: 0x%p", caller_regs.ft11);
      return os;
    }
  };

  /**
   * @brief 被调用者保存寄存器
   * @see riscv-abi.pdf#1.1
   * @see riscv-abi.pdf#1.2
   */
  struct CalleeRegs {
    uintptr_t sp;
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;
    uintptr_t fs0;
    uintptr_t fs1;
    uintptr_t fs2;
    uintptr_t fs3;
    uintptr_t fs4;
    uintptr_t fs5;
    uintptr_t fs6;
    uintptr_t fs7;
    uintptr_t fs8;
    uintptr_t fs9;
    uintptr_t fs10;
    uintptr_t fs11;
    friend std::ostream &operator<<(std::ostream &os,
                                    const CalleeRegs &callee_regs) {
      printf("sp: 0x%p, ", callee_regs.sp);
      printf("s0: 0x%p, ", callee_regs.s0);
      printf("s1: 0x%p, ", callee_regs.s1);
      printf("s2: 0x%p\n", callee_regs.s2);
      printf("s3: 0x%p, ", callee_regs.s3);
      printf("s4: 0x%p, ", callee_regs.s4);
      printf("s5: 0x%p, ", callee_regs.s5);
      printf("s6: 0x%p\n", callee_regs.s6);
      printf("s7: 0x%p, ", callee_regs.s7);
      printf("s8: 0x%p, ", callee_regs.s8);
      printf("s9: 0x%p, ", callee_regs.s9);
      printf("s10: 0x%p\n", callee_regs.s10);
      printf("s11: 0x%p, ", callee_regs.s11);
      printf("fs0: 0x%p, ", callee_regs.fs0);
      printf("fs1: 0x%p, ", callee_regs.fs1);
      printf("fs2: 0x%p\n", callee_regs.fs2);
      printf("fs3: 0x%p, ", callee_regs.fs3);
      printf("fs4: 0x%p, ", callee_regs.fs4);
      printf("fs5: 0x%p, ", callee_regs.fs5);
      printf("fs6: 0x%p\n", callee_regs.fs6);
      printf("fs7: 0x%p, ", callee_regs.fs7);
      printf("fs8: 0x%p, ", callee_regs.fs8);
      printf("fs9: 0x%p, ", callee_regs.fs9);
      printf("fs10: 0x%p\n", callee_regs.fs10);
      printf("fs11: 0x%p", callee_regs.fs11);
      return os;
    }
  };

  /**
   * @brief 所有寄存器，在中断时使用，共 32+32+7=71 个
   */
  struct AllRegs {
    Xregs xregs;
    Fregs fregs;
    uintptr_t sepc;
    uintptr_t stval;
    Xcause xcause;
    Xie xie;
    Xstatus xstatus;
    Satp satp;
    uintptr_t sscratch;
    // friend std::ostream &operator<<(std::ostream &_os,
    //                                 const AllRegs &all_regs) {
    //   ()all_regs.fregs;
    //   _os << all_regs.xregs << std::endl;
    //   printf(
    //       "sepc: 0x%p, stval: 0x%p, scause: 0x%p, sie: 0x%p, xstatus: "
    //       "0x%p, satp: 0x%p, sscratch: 0x%p",
    //       all_regs.sepc, all_regs.stval, all_regs.scause, all_regs.sie,
    //       all_regs.xstatus_.val, all_regs.satp.val, all_regs.sscratch);
    //   return _os;
    // }
  };

  /**
   * @brief 读取所有寄存器
   * @param  all_regs        要保存读取到的的值
   */
  static inline void ReadAllRegs(AllRegs &all_regs) {
    asm("mv %0, zero" : "=r"(all_regs.xregs.zero));
    asm("mv %0, ra" : "=r"(all_regs.xregs.ra));
    asm("mv %0, sp" : "=r"(all_regs.xregs.sp));
    asm("mv %0, gp" : "=r"(all_regs.xregs.gp));
    asm("mv %0, tp" : "=r"(all_regs.xregs.tp));
    asm("mv %0, t0" : "=r"(all_regs.xregs.t0));
    asm("mv %0, t1" : "=r"(all_regs.xregs.t1));
    asm("mv %0, t2" : "=r"(all_regs.xregs.t2));
    asm("mv %0, s0" : "=r"(all_regs.xregs.s0));
    asm("mv %0, s1" : "=r"(all_regs.xregs.s1));
    asm("mv %0, a0" : "=r"(all_regs.xregs.a0));
    asm("mv %0, a1" : "=r"(all_regs.xregs.a1));
    asm("mv %0, a2" : "=r"(all_regs.xregs.a2));
    asm("mv %0, a3" : "=r"(all_regs.xregs.a3));
    asm("mv %0, a4" : "=r"(all_regs.xregs.a4));
    asm("mv %0, a5" : "=r"(all_regs.xregs.a5));
    asm("mv %0, a6" : "=r"(all_regs.xregs.a6));
    asm("mv %0, a7" : "=r"(all_regs.xregs.a7));
    asm("mv %0, s2" : "=r"(all_regs.xregs.s2));
    asm("mv %0, s3" : "=r"(all_regs.xregs.s3));
    asm("mv %0, s4" : "=r"(all_regs.xregs.s4));
    asm("mv %0, s5" : "=r"(all_regs.xregs.s5));
    asm("mv %0, s6" : "=r"(all_regs.xregs.s6));
    asm("mv %0, s7" : "=r"(all_regs.xregs.s7));
    asm("mv %0, s8" : "=r"(all_regs.xregs.s8));
    asm("mv %0, s9" : "=r"(all_regs.xregs.s9));
    asm("mv %0, s10" : "=r"(all_regs.xregs.s10));
    asm("mv %0, s11" : "=r"(all_regs.xregs.s11));
    asm("mv %0, t3" : "=r"(all_regs.xregs.t3));
    asm("mv %0, t4" : "=r"(all_regs.xregs.t4));
    asm("mv %0, t5" : "=r"(all_regs.xregs.t5));
    asm("mv %0, t6" : "=r"(all_regs.xregs.t6));

    //    asm("mv %0, ft0" : "=r"(all_regs.fregs.ft0));
    //    asm("mv %0, ft1" : "=r"(all_regs.fregs.ft1));
    //    asm("mv %0, ft2" : "=r"(all_regs.fregs.ft2));
    //    asm("mv %0, ft3" : "=r"(all_regs.fregs.ft3));
    //    asm("mv %0, ft4" : "=r"(all_regs.fregs.ft4));
    //    asm("mv %0, ft5" : "=r"(all_regs.fregs.ft5));
    //    asm("mv %0, ft6" : "=r"(all_regs.fregs.ft6));
    //    asm("mv %0, ft7" : "=r"(all_regs.fregs.ft7));
    //    asm("mv %0, fs0" : "=r"(all_regs.fregs.fs0));
    //    asm("mv %0, fs1" : "=r"(all_regs.fregs.fs1));
    //    asm("mv %0, fa0" : "=r"(all_regs.fregs.fa0));
    //    asm("mv %0, fa1" : "=r"(all_regs.fregs.fa1));
    //    asm("mv %0, fa2" : "=r"(all_regs.fregs.fa2));
    //    asm("mv %0, fa3" : "=r"(all_regs.fregs.fa3));
    //    asm("mv %0, fa4" : "=r"(all_regs.fregs.fa4));
    //    asm("mv %0, fa5" : "=r"(all_regs.fregs.fa5));
    //    asm("mv %0, fa6" : "=r"(all_regs.fregs.fa6));
    //    asm("mv %0, fa7" : "=r"(all_regs.fregs.fa7));
    //    asm("mv %0, fs2" : "=r"(all_regs.fregs.fs2));
    //    asm("mv %0, fs3" : "=r"(all_regs.fregs.fs3));
    //    asm("mv %0, fs4" : "=r"(all_regs.fregs.fs4));
    //    asm("mv %0, fs5" : "=r"(all_regs.fregs.fs5));
    //    asm("mv %0, fs6" : "=r"(all_regs.fregs.fs6));
    //    asm("mv %0, fs7" : "=r"(all_regs.fregs.fs7));
    //    asm("mv %0, fs8" : "=r"(all_regs.fregs.fs8));
    //    asm("mv %0, fs9" : "=r"(all_regs.fregs.fs9));
    //    asm("mv %0, fs10" : "=r"(all_regs.fregs.fs10));
    //    asm("mv %0, fs11" : "=r"(all_regs.fregs.fs11));
    //    asm("mv %0, ft8" : "=r"(all_regs.fregs.ft8));
    //    asm("mv %0, ft9" : "=r"(all_regs.fregs.ft9));
    //    asm("mv %0, ft10" : "=r"(all_regs.fregs.ft10));
    //    asm("mv %0, ft11" : "=r"(all_regs.fregs.ft11));

    asm("csrr %0, sepc" : "=r"(all_regs.sepc));
    asm("csrr %0, stval" : "=r"(all_regs.stval));
    asm("csrr %0, scause" : "=r"(all_regs.xcause));
    asm("csrr %0, xstatus" : "=r"(all_regs.xstatus));
    asm("csrr %0, sscratch" : "=r"(all_regs.sscratch));

    return;
  }

  /**
   * @brief 上下文，用于任务切换
   * @note caller_regs 由编译器保存/恢复
   */
  struct Context {
    /// 运行此任务的 core id
    uintptr_t coreid;
    uintptr_t ra;
    CalleeRegs callee_regs;
    Satp satp;
    uintptr_t sepc;
    Xstatus xstatus;
    Xie xie;
    Xip xip;
    uintptr_t sscratch;
    // friend std::ostream &operator<<(std::ostream &_os,
    //                                 const Context &_context) {
    //   printf("coreid: 0x%X, ", _context.coreid);
    //   printf("ra: 0x%p\n", _context.ra);
    //   std::cout << _context.callee_regs << std::endl;
    //   printf("satp: 0x%p, ", _context.satp.val);
    //   printf("sepc: 0x%p, ", _context.sepc);
    //   printf("xstatus: 0x%p, ", _context.xstatus_.val);
    //   printf("sie: 0x%p, ", _context.sie);
    //   printf("sip: 0x%p, ", _context.sip);
    //   printf("sscratch: 0x%p", _context.sscratch);
    //   return _os;
    // }
  };

  /**
   * @brief 读取 sstatus 寄存器
   * @return uint64_t         读取到的值
   */
  static inline Xstatus ReadSstatus() {
    Xstatus x;
    asm("csrr %0, sstatus" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sstatus 寄存器
   * @param  x                要写的值
   */
  static inline void WriteSstatus(Xstatus x) {
    asm("csrw sstatus, %0" : : "r"(x));
  }

  /**
   * @brief 读 sip
   * @return Xip 读取到的值
   * @note Supervisor Interrupt Pending
   */
  static inline Xip ReadSip() {
    Xip x;
    asm("csrr %0, sip" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sip
   * @param  x               要写的值
   */
  static inline void WriteSip(Xip x) { asm("csrw sip, %0" : : "r"(x)); }

  /**
   * @brief 读 sie
   * @return Xie         读到的值
   */
  static inline Xie ReadSie() {
    Xie x;
    asm("csrr %0, sie" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sie
   * @param  x                要写的值
   */
  static inline void WriteSie(Xie x) { asm("csrw sie, %0" : : "r"(x)); }

  /**
   * @brief 读 sepc
   * @return uint64_t         读到的值
   * @note machine exception program counter, holds the instruction address to
   * which a return from exception will go.
   */
  static inline uint64_t ReadSepc() {
    uint64_t x;
    asm("csrr %0, sepc" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sepc
   * @param  x               要写的值
   */
  static inline void WriteSepc(uint64_t x) { asm("csrw sepc, %0" : : "r"(x)); }

  /**
   * @brief 读 stvec
   * @return Xtvec 读到的值
   */
  static inline Xtvec ReadStvec() {
    Xtvec x;
    asm("csrr %0, stvec" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 stvec
   * @param  x               要写的值
   */
  static inline void WriteStvec(Xtvec x) { asm("csrw stvec, %0" : : "r"(x)); }

  /**
   * @brief 读 sscratch 寄存器
   * @param  x                要写的值
   */
  static inline uint64_t ReadSscratch() {
    uint64_t x;
    asm("csrr %0, sscratch" : "=r"(x));
    return x;
  }

  /**
   * @brief 写 sscratch 寄存器
   * @param  x                要写的值
   */
  static inline void WriteSscratch(uint64_t x) {
    asm("csrw sscratch, %0" : : "r"(x));
  }

  /**
   * @brief 读 scause 寄存器 Supervisor Trap Cause
   * @return Xcause         读到的值
   */
  static inline Xcause ReadScause() {
    Xcause x;
    asm("csrr %0, scause" : "=r"(x));
    return x;
  }

  /**
   * @brief 读 stval 寄存器 Supervisor Trap Value
   * @return uint64_t         读到的值
   */
  static inline uint64_t ReadStval() {
    uint64_t x;
    asm("csrr %0, stval" : "=r"(x));
    return x;
  }

  /**
   * @brief 读 time 寄存器 supervisor-mode cycle counter
   * @return uint64_t         读到的值
   */
  static inline uint64_t ReadTime() {
    uint64_t x;
    // asm ("csrr %0, time" : "=r" (x) );
    // this instruction will trap in SBI
    asm("rdtime %0" : "=r"(x));
    return x;
  }

  /**
   * @brief 设置中断模式，直接
   */
  static inline void SetStvecDirect() {
    auto stvec = ReadStvec();
    stvec.xtvec_.mode = Xtvec::kDirect;
    WriteStvec(stvec);
  }

  /**
   * @brief 设置中断模式，直接
   * @param addr 处理函数的地址
   */
  static inline void SetStvecDirect(uint64_t addr) {
    auto stvec = ReadStvec();
    stvec.xtvec_.mode = Xtvec::kDirect;
    stvec.xtvec_.base = addr >> 2;
    WriteStvec(stvec);
  }

  /**
   * @brief 设置中断模式，向量
   */
  static inline void SetStvecVectored() {
    auto stvec = ReadStvec();
    stvec.xtvec_.mode = Xtvec::kVectored;
    WriteStvec(stvec);
  }

  /**
   * @brief 允许中断
   */
  static inline void EnableSupervisorIntr() {
    WriteSstatus(Xstatus(ReadSstatus().val_ | Xstatus::kXstatusSie));
  }

  /**
   * @brief 禁止中断
   */
  static inline void DisableIntr() {
    WriteSstatus(Xstatus(ReadSstatus().val_ & ~Xstatus::kXstatusSie));
  }

  /**
   * @brief 允许软件中断
   */
  static inline void EnableSupervisorSoftware() {
    WriteSie(Xie(ReadSie().val_ | Xie::kSsie));
  }

  /**
   * @brief 禁止软件中断
   */
  static inline void DisableSupervisorSoftware() {
    WriteSie(Xie(ReadSie().val_ & ~Xie::kSsie));
  }

  /**
   * @brief 允许定时器中断
   */
  static inline void EnableSupervisorTimer() {
    WriteSie(Xie(ReadSie().val_ | Xie::kStie));
  }

  /**
   * @brief 禁止定时器中断
   */
  static inline void DisableSupervisorTimer() {
    WriteSie(Xie(ReadSie().val_ & ~Xie::kStie));
  }

  /**
   * @brief 允许外部中断
   */
  static inline void EnableSupervisorExternal() {
    WriteSie(Xie(ReadSie().val_ | Xie::kSeie));
  }

  /**
   * @brief 禁止外部中断
   */
  static inline void DisableSupervisorExternal() {
    WriteSie(Xie(ReadSie().val_ & ~Xie::kSeie));
  }
};

/// 立即数掩码，大于这个值需要使用寄存器中转
static constexpr uint64_t kCsrImmOpMask = 0x1F;

struct CsrRegInfoBase {
  using DataType = uint64_t;
  static constexpr uint64_t kBitOffset = 0;
  static constexpr uint64_t kBitWidth = 64;
  static constexpr uint64_t kBitMask = ~0;
  static constexpr uint64_t kAllSetMask = ~0;
};

/// @todo 确认所有 info 信息的掩码
struct SscratchInfo : public CsrRegInfoBase {};

struct SepcInfo : public CsrRegInfoBase {};

struct ScauseInfo : public CsrRegInfoBase {
  struct Interrupt {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 63;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 1UL << 63;
    static constexpr uint64_t kAllSetMask = 1;
  };

  struct ExceptionCode {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 63;
    static constexpr uint64_t kBitMask = ~(1UL << 63);
    static constexpr uint64_t kAllSetMask = ~(1UL << 63);
  };
};

struct SstatusInfo : public CsrRegInfoBase {
  /** Parameter data for sie */
  struct Sie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 1;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x2;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
  /** Parameter data for spie */
  struct Spie {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 5;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x20;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
  /** Parameter data for spp */
  struct Spp {
    using DataType = bool;
    static constexpr uint64_t kBitOffset = 8;
    static constexpr uint64_t kBitWidth = 1;
    static constexpr uint64_t kBitMask = 0x100;
    static constexpr uint64_t kAllSetMask = 0x1;
  };
};

struct StvecInfo : public CsrRegInfoBase {
  /** Parameter data for base */
  struct Base {
    using DataType = uint64_t;
    static constexpr uint64_t kBitOffset = 2;
    static constexpr uint64_t kBitWidth = 62;
    static constexpr uint64_t kBitMask = ~0x3;
    static constexpr uint64_t kAllSetMask = ~0x3;
  };
  /** Parameter data for mode */
  struct Mode {
    using DataType = uint8_t;
    static constexpr uint64_t kBitOffset = 0;
    static constexpr uint64_t kBitWidth = 2;
    static constexpr uint64_t kBitMask = 0x3;
    static constexpr uint64_t kAllSetMask = 0x3;
  };
};

struct SidelegInfo : public CsrRegInfoBase {};

struct SedelegInfo : public CsrRegInfoBase {};

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
   * 读 csr 寄存器
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t Read() {
    uint64_t value = -1;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrr %0, sscratch" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrr %0, sepc" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrr %0, scause" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrr %0, sstatus" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrr %0, stvec" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrr %0, sideleg" : "=r"(value) : :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrr %0, sedeleg" : "=r"(value) : :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * () 重载
   */
  static inline uint64_t operator()() { return Read(); }
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
   * 写 csr 寄存器
   * @param value 要写的值
   */
  static inline void Write(uint64_t value) {
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrw sscratch, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrw sepc, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrw scause, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrw sstatus, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrw stvec, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrw sideleg, %0" : : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrw sedeleg, %0" : : "r"(value) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 写 csr 寄存器，不通过寄存器中转
   * @param value 要写的值
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static inline void WriteImm(const uint8_t value) {
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrwi sscratch, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrwi sepc, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrwi scause, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrwi sstatus, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrwi stvec, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrwi sideleg, %0" : : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrwi sedeleg, %0" : : "i"(value) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后写寄存器
   * @param value 要写的值
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t ReadWrite(uint64_t value) {
    uint64_t old_value;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrw %0, sscratch, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrw %0, sepc, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrw %0, scause, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrw %0, sstatus, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrw %0, stvec, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrw %0, sideleg, %1" : "=r"(old_value) : "r"(value) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrw %0, sedeleg, %1" : "=r"(old_value) : "r"(value) :);
    } else {
      printf("error\n");
    }
    return old_value;
  }

  /**
   * 先读后写寄存器，不通过寄存器中转
   * @param value 要写的值
   * @return uint64_t 寄存器的值
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static inline uint64_t ReadWriteImm(const uint8_t value) {
    uint64_t old_value;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrwi %0, sscratch, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrwi %0, sepc, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrwi %0, scause, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrwi %0, sstatus, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrwi %0, stvec, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrwi %0, sideleg, %1" : "=r"(old_value) : "i"(value) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrwi %0, sedeleg, %1" : "=r"(old_value) : "i"(value) :);
    } else {
      printf("error\n");
    }
    return old_value;
  }

  /**
   * 通过掩码设置寄存器
   * @param mask 掩码
   */
  static inline void SetBits(uint64_t mask) {
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrs zero, sscratch, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrs zero, sepc, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrs zero, scause, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrs zero, sstatus, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrs zero, stvec, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrs zero, sideleg, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrs zero, sedeleg, %0" : : "r"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后通过掩码设置寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t ReadSetBits(uint64_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrs %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrs %0, sepc, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrs %0, scause, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrs %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrs %0, stvec, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrs %0, sideleg, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrs %0, sedeleg, %1" : "=r"(value) : "r"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 清零寄存器
   * @param mask 掩码
   */
  static inline void ClearBits(uint64_t mask) {
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrc zero, sscratch, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrc zero, sepc, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrc zero, scause, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrc zero, sstatus, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrc zero, stvec, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrc zero, sideleg, %0" : : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrc zero, sedeleg, %0" : : "r"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后清零寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t ReadClearBits(uint64_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrc %0, sscratch, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrc %0, sepc, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrc %0, scause, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrc %0, sstatus, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrc %0, stvec, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrc %0, sideleg, %1" : "=r"(value) : "r"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrc %0, sedeleg, %1" : "=r"(value) : "r"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 通过掩码设置寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static inline void SetBitsImm(const uint8_t mask) {
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrsi zero, sscratch, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrsi zero, sepc, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrsi zero, scause, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrsi zero, sstatus, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrsi zero, stvec, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrsi zero, sideleg, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrsi zero, sedeleg, %0" : : "i"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后通过掩码设置寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static inline uint64_t ReadSetBitsImm(const uint8_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrsi %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrsi %0, sepc, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrsi %0, scause, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrsi %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrsi %0, stvec, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrsi %0, sideleg, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrsi %0, sedeleg, %1" : "=r"(value) : "i"(mask) :);
    } else {
      printf("error\n");
    }
    return value;
  }

  /**
   * 清零寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static inline void ClearBitsImm(const uint8_t mask) {
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrci zero, sscratch, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrci zero, sepc, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrci zero, scause, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrci zero, sstatus, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrci zero, stvec, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrci zero, sideleg, %0" : : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrci zero, sedeleg, %0" : : "i"(mask) :);
    } else {
      printf("error\n");
    }
  }

  /**
   * 先读后清零寄存器，不通过寄存器中转
   * @param mask 掩码
   * @note 只能写 kCsrImmOpMask 范围内的值
   */
  static inline uint64_t ReadClearBitsImm(const uint8_t mask) {
    uint64_t value;
    if constexpr (std::is_same<Reg, SscratchInfo>::value) {
      asm("csrrci %0, sscratch, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SepcInfo>::value) {
      asm("csrrci %0, sepc, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, ScauseInfo>::value) {
      asm("csrrci %0, scause, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SstatusInfo>::value) {
      asm("csrrci %0, sstatus, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, StvecInfo>::value) {
      asm("csrrci %0, stvec, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SidelegInfo>::value) {
      asm("csrrci %0, sideleg, %1" : "=r"(value) : "i"(mask) :);
    } else if constexpr (std::is_same<Reg, SedelegInfo>::value) {
      asm("csrrci %0, sedeleg, %1" : "=r"(value) : "i"(mask) :);
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
    if constexpr ((value & kCsrImmOpMask) == value) {
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
    if constexpr ((mask & kCsrImmOpMask) == mask) {
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
    if constexpr ((mask & kCsrImmOpMask) == mask) {
      ClearBitsImm(mask);
    } else {
      ClearBits(mask);
    }
  }

  /**
   * |= 重载
   */
  inline void operator|=(uint64_t mask) { SetBits(mask); }
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
    if constexpr ((value & kCsrImmOpMask) == value) {
      return ReadWriteRegBase<Reg>::ReadWriteImm(value);
    } else {
      return ReadWrite(value);
    }
  }

  /**
   * 先读后写寄存器
   * @tparam value 要写的值
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t ReadWrite(uint64_t value) { return ReadWrite(value); }

  /**
   * 通过常数掩码先读后写寄存器
   * @tparam mask 掩码
   * @return uint64_t 寄存器的值
   */
  template <uint64_t mask>
  static uint64_t ReadSetBitsConst() {
    if constexpr ((mask & kCsrImmOpMask) == mask) {
      return WriteOnlyRegBase<Reg>::ReadSetBitsImm(mask);
    } else {
      return ReadSetBits(mask);
    }
  }

  /**
   * 通过掩码先读后写寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t ReadSetBits(uint64_t mask) {
    return ReadSetBits(mask);
  }

  /**
   * 通过常数掩码先读后清零寄存器
   * @tparam mask 掩码
   * @return uint64_t 寄存器的值
   */
  template <uint64_t mask>
  static uint64_t ReadClearBitsConst() {
    if constexpr ((mask & kCsrImmOpMask) == mask) {
      return WriteOnlyRegBase<Reg>::ReadClearBitsImm(mask);
    } else {
      return ReadClearBits(mask);
    }
  }

  /**
   * 通过掩码先读后清零寄存器
   * @param mask 掩码
   * @return uint64_t 寄存器的值
   */
  static inline uint64_t ReadClearBits(uint64_t mask) {
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
   * @return Info::DataType 指定位值
   */
  static inline Info::DataType Get() {
    return (typename Info::DataType)((Reg::Read() & Info::kBitMask) >>
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
  static inline void Set() {
    if constexpr ((Info::kBitMask & kCsrImmOpMask) == Info::kBitMask) {
      Reg::SetBitsImm(Info::kBitMask);
    } else {
      Reg::SetBits(Info::kBitMask);
    }
  }

  /**
   * 清零对应 Reg 的由 Info 规定的指定位
   */
  static inline void Clear() {
    if constexpr ((Info::kBitMask & kCsrImmOpMask) == Info::kBitMask) {
      Reg::ClearBitsImm(Info::kBitMask);
    } else {
      Reg::ClearBits(Info::kBitMask);
    }
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
  static inline void Write(Info::DataType value) {
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
  static inline Info::DataType ReadWrite(Info::DataType value) {
    auto org_value = Reg::Read();
    auto new_value = (org_value & ~Info::kBitMask) |
                     ((value << Info::kBitOffset) & Info::kBitMask);
    Reg::Write(new_value);
    return (Info::DataType)((org_value & Info::kBitMask) >> Info::kBitOffset);
  }
};

class Sscratch : public ReadWriteRegBase<SscratchInfo> {
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
};

class Sepc : public ReadWriteRegBase<SepcInfo> {
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
};

class Scause : public ReadWriteRegBase<ScauseInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Scause() = default;
  Scause(const Scause &) = delete;
  Scause(Scause &&) = delete;
  auto operator=(const Scause &) -> Scause & = delete;
  auto operator=(Scause &&) -> Scause & = delete;
  virtual ~Scause() = default;
  /// @}

  ReadWriteField<ReadWriteRegBase<ScauseInfo>, ScauseInfo::Interrupt> interrupt;
  ReadWriteField<ReadWriteRegBase<ScauseInfo>, ScauseInfo::ExceptionCode>
      exception_code;
};

class Sstatus : public ReadWriteRegBase<SstatusInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Sstatus() = default;
  Sstatus(const Sstatus &) = delete;
  Sstatus(Sstatus &&) = delete;
  auto operator=(const Sstatus &) -> Sstatus & = delete;
  auto operator=(Sstatus &&) -> Sstatus & = delete;
  virtual ~Sstatus() = default;
  /// @}

  ReadWriteField<ReadWriteRegBase<SstatusInfo>, SstatusInfo::Sie> sie;
  ReadWriteField<ReadWriteRegBase<SstatusInfo>, SstatusInfo::Spie> spie;
  ReadWriteField<ReadWriteRegBase<SstatusInfo>, SstatusInfo::Spp> spp;
};

class Stvec : public ReadWriteRegBase<StvecInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Stvec() = default;
  Stvec(const Stvec &) = delete;
  Stvec(Stvec &&) = delete;
  auto operator=(const Stvec &) -> Stvec & = delete;
  auto operator=(Stvec &&) -> Stvec & = delete;
  virtual ~Stvec() = default;
  /// @}

  ReadWriteField<ReadWriteRegBase<StvecInfo>, StvecInfo::Base> base;
  ReadWriteField<ReadWriteRegBase<StvecInfo>, StvecInfo::Mode> mode;
};

class Sideleg : public ReadWriteRegBase<SidelegInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Sideleg() = default;
  Sideleg(const Sideleg &) = delete;
  Sideleg(Sideleg &&) = delete;
  auto operator=(const Sideleg &) -> Sideleg & = delete;
  auto operator=(Sideleg &&) -> Sideleg & = delete;
  virtual ~Sideleg() = default;
  /// @}
};

class Sedeleg : public ReadWriteRegBase<SedelegInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Sedeleg() = default;
  Sedeleg(const Sedeleg &) = delete;
  Sedeleg(Sedeleg &&) = delete;
  auto operator=(const Sedeleg &) -> Sedeleg & = delete;
  auto operator=(Sedeleg &&) -> Sedeleg & = delete;
  virtual ~Sedeleg() = default;
  /// @}
};

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_

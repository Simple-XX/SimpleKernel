
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

#include "iostream"
#include "stdio.h"

/**
 * riscv64 cpu 相关定义
 * @see riscv-privileged-v1.10.pd
 * https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf
 */
class Cpu {
 public:
  /**
   * Privilege Levels
   * @see riscv-privileged-v1.10.pd#1.3
   */
  enum {
    kPrivilegeLevelUser = 0,
    kPrivilegeLevelSuper = 1,
    kPrivilegeLevelMachine = 3,
  };

  /**
   * @brief Xstatus 寄存器定义
   * @see riscv-privileged-v1.10.pdf#3.1.6
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
        // The UIE bit enables or disables user-mode interrupts.
        uint64_t uie : 1;
        // The SIE bit enables or disables all interrupts in supervisor mode.
        uint64_t sie : 1;
        // Reserved Writes Preserve Values, Reads Ignore Values 1
        uint64_t wpri1 : 1;
        // The MIE bit enables or disables all interrupts in machine mode.
        uint64_t mie : 1;
        // The UPIE bit indicates whether user-level interrupts were enabled
        // prior to taking a user-level trap.
        uint64_t upie : 1;
        // The SPIE bit indicates whether supervisor interrupts were enabled
        // prior to trapping into supervisor mode.
        uint64_t spie : 1;
        uint64_t wpri2 : 1;
        uint64_t mpie : 1;
        // The SPP bit indicates the privilege level at which a hart was
        // executing before entering supervisor mode.
        // xPP holds the previous privilege mode.
        uint64_t spp : 1;
        uint64_t wpri3 : 2;
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
        uint64_t wpri5 : 27;
        // The SD bit is read-only and is set when either the FS or XS bits
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
      printf("val: 0x%p, uie: %s, sie: %s, upie: %s, spie: %s, spp: %s",
             xstatus.val_,
             (xstatus.xstatus_.uie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.sie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.upie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.spie == true ? "Enable" : "Disable"),
             (xstatus.xstatus_.spp == true ? "S Mode" : "U Mode")

      );
      return os;
    }
  };

  /**
   * @brief Xtvec 寄存器定义
   * @see riscv-privileged-v1.10.pdf#3.1.12
   */
  class Xtvec {
   public:
    /// 中断模式 直接
    static constexpr const uint64_t kDirect = 0x0;
    /// 中断模式 向量
    static constexpr const uint64_t kVectored = 0x1;

    union {
      struct {
        uint64_t base : 62;
        uint64_t mode : 2;
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
   * @see riscv-privileged-v1.10.pdf#3.1.14
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
        /// User-level Software Interrupt Pending
        uint64_t usip : 1;
        /// Supervisor Software Interrupt Pending
        uint64_t ssip : 1;
        uint64_t wpri1 : 1;
        uint64_t msip : 1;
        /// User Timer Interrupt Pending
        uint64_t utip : 1;
        /// Supervisor Timer Interrupt Pending
        uint64_t stip : 1;
        uint64_t wpri2 : 1;
        /// Machine Timer Interrupt Pending
        uint64_t mtip : 1;
        /// User External Interrupt Pending
        uint64_t ueip : 1;
        /// Supervisor External Interrupt Pending
        uint64_t seip : 1;
        uint64_t wpri3 : 1;
        /// Machine External Interrupt Pending
        uint64_t meip : 1;
        /// wpri
        uint64_t wpri4 : 52;
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
   * @see riscv-privileged-v1.10.pdf#3.1.14
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
        /// User-level Software Interrupt Enable
        uint64_t usie : 1;
        /// Supervisor Software Interrupt Enable
        uint64_t ssie : 1;
        uint64_t wpri1 : 1;
        uint64_t msie : 1;
        /// User Timer Interrupt Enable
        uint64_t utie : 1;
        /// Supervisor Timer Interrupt Enable
        uint64_t stie : 1;
        uint64_t wpri2 : 1;
        /// Machine Timer Interrupt Enable
        uint64_t mtie : 1;
        /// User External Interrupt Enable
        uint64_t ueie : 1;
        /// Supervisor External Interrupt Enable
        uint64_t seie : 1;
        uint64_t wpri3 : 1;
        /// Machine External Interrupt Enable
        uint64_t meie : 1;
        /// wpri
        uint64_t wpri4 : 52;
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
   * @see riscv-privileged-v1.10.pdf#3.1.20
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
   * @see riscv-privileged-v1.10.pdf#4.1.12
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
    uintptr_t zero;
    uintptr_t ra;
    uintptr_t sp;
    uintptr_t gp;
    uintptr_t tp;
    uintptr_t t0;
    uintptr_t t1;
    uintptr_t t2;
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t a0;
    uintptr_t a1;
    uintptr_t a2;
    uintptr_t a3;
    uintptr_t a4;
    uintptr_t a5;
    uintptr_t a6;
    uintptr_t a7;
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
    uintptr_t t3;
    uintptr_t t4;
    uintptr_t t5;
    uintptr_t t6;
    // friend std::ostream &operator<<(std::ostream &_os, const Xregs &_xregs)
    // {
    //   printf("zero: 0x%p, ", _xregs.zero);
    //   printf("ra: 0x%p, ", _xregs.ra);
    //   printf("sp: 0x%p, ", _xregs.sp);
    //   printf("gp: 0x%p\n", _xregs.gp);
    //   printf("tp: 0x%p, ", _xregs.tp);
    //   printf("t0: 0x%p, ", _xregs.t0);
    //   printf("t1: 0x%p, ", _xregs.t1);
    //   printf("t2: 0x%p\n", _xregs.t2);
    //   printf("s0: 0x%p, ", _xregs.s0);
    //   printf("s1: 0x%p, ", _xregs.s1);
    //   printf("a0: 0x%p, ", _xregs.a0);
    //   printf("a1: 0x%p\n", _xregs.a1);
    //   printf("a2: 0x%p, ", _xregs.a2);
    //   printf("a3: 0x%p, ", _xregs.a3);
    //   printf("a4: 0x%p, ", _xregs.a4);
    //   printf("a5: 0x%p\n", _xregs.a5);
    //   printf("a6: 0x%p, ", _xregs.a6);
    //   printf("a7: 0x%p, ", _xregs.a7);
    //   printf("s2: 0x%p, ", _xregs.s2);
    //   printf("s3: 0x%p\n", _xregs.s3);
    //   printf("s4: 0x%p, ", _xregs.s4);
    //   printf("s5: 0x%p, ", _xregs.s5);
    //   printf("s6: 0x%p, ", _xregs.s6);
    //   printf("s7: 0x%p\n", _xregs.s7);
    //   printf("s8: 0x%p, ", _xregs.s8);
    //   printf("s9: 0x%p, ", _xregs.s9);
    //   printf("s10: 0x%p, ", _xregs.s10);
    //   printf("s11: 0x%p\n", _xregs.s11);
    //   printf("t3: 0x%p, ", _xregs.t3);
    //   printf("t4: 0x%p, ", _xregs.t4);
    //   printf("t5: 0x%p, ", _xregs.t5);
    //   printf("t6: 0x%p", _xregs.t6);
    //   return _os;
    // }
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
    // friend std::ostream &operator<<(std::ostream &_os, const Fregs &_fregs)
    // {
    //   printf("ft0: 0x%p, ", _fregs.ft0);
    //   printf("ft1: 0x%p, ", _fregs.ft1);
    //   printf("ft2: 0x%p, ", _fregs.ft2);
    //   printf("ft3: 0x%p\n", _fregs.ft3);
    //   printf("ft4: 0x%p, ", _fregs.ft4);
    //   printf("ft5: 0x%p, ", _fregs.ft5);
    //   printf("ft6: 0x%p, ", _fregs.ft6);
    //   printf("ft7: 0x%p\n", _fregs.ft7);
    //   printf("fs0: 0x%p, ", _fregs.fs0);
    //   printf("fs1: 0x%p, ", _fregs.fs1);
    //   printf("fa0: 0x%p, ", _fregs.fa0);
    //   printf("fa1: 0x%p\n", _fregs.fa1);
    //   printf("fa2: 0x%p, ", _fregs.fa2);
    //   printf("fa3: 0x%p, ", _fregs.fa3);
    //   printf("fa4: 0x%p, ", _fregs.fa4);
    //   printf("fa5: 0x%p\n", _fregs.fa5);
    //   printf("fa6: 0x%p, ", _fregs.fa6);
    //   printf("fa7: 0x%p, ", _fregs.fa7);
    //   printf("fs2: 0x%p, ", _fregs.fs2);
    //   printf("fs3: 0x%p\n", _fregs.fs3);
    //   printf("fs4: 0x%p, ", _fregs.fs4);
    //   printf("fs5: 0x%p, ", _fregs.fs5);
    //   printf("fs6: 0x%p, ", _fregs.fs6);
    //   printf("fs7: 0x%p\n", _fregs.fs7);
    //   printf("fs8: 0x%p, ", _fregs.fs8);
    //   printf("fs9: 0x%p, ", _fregs.fs9);
    //   printf("fs10: 0x%p, ", _fregs.fs10);
    //   printf("fs11: 0x%p\n", _fregs.fs11);
    //   printf("ft8: 0x%p, ", _fregs.ft8);
    //   printf("ft9: 0x%p, ", _fregs.ft9);
    //   printf("ft10: 0x%p, ", _fregs.ft10);
    //   printf("ft11: 0x%p", _fregs.ft11);
    //   return _os;
    // }
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
    //   (void)all_regs.fregs;
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

    asm volatile("csrr %0, sepc" : "=r"(all_regs.sepc));
    asm volatile("csrr %0, stval" : "=r"(all_regs.stval));
    asm volatile("csrr %0, scause" : "=r"(all_regs.xcause));
    asm volatile("csrr %0, xstatus" : "=r"(all_regs.xstatus));
    asm volatile("csrr %0, sscratch" : "=r"(all_regs.sscratch));

    return;
  }

  /**
   * @brief 调用者保存寄存器
   */
  struct CallerRegs {
    uintptr_t ra;
    uintptr_t t0;
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
    // friend std::ostream &operator<<(std::ostream &_os,
    //                                 const Caller_Regs &_caller_regs) {
    //   printf("ra: 0x%p, ", _caller_regs.ra);
    //   printf("t0: 0x%p, ", _caller_regs.t0);
    //   printf("t2: 0x%p, ", _caller_regs.t2);
    //   printf("a0: 0x%p\n", _caller_regs.a0);
    //   printf("a1: 0x%p, ", _caller_regs.a1);
    //   printf("a2: 0x%p, ", _caller_regs.a2);
    //   printf("a3: 0x%p, ", _caller_regs.a3);
    //   printf("a4: 0x%p\n", _caller_regs.a4);
    //   printf("a5: 0x%p, ", _caller_regs.a5);
    //   printf("a6: 0x%p, ", _caller_regs.a6);
    //   printf("a7: 0x%p, ", _caller_regs.a7);
    //   printf("t3: 0x%p\n", _caller_regs.t3);
    //   printf("t4: 0x%p, ", _caller_regs.t4);
    //   printf("t5: 0x%p, ", _caller_regs.t5);
    //   printf("t6: 0x%p, ", _caller_regs.t6);
    //   printf("ft0: 0x%p\n", _caller_regs.ft0);
    //   printf("ft1: 0x%p, ", _caller_regs.ft1);
    //   printf("ft2: 0x%p, ", _caller_regs.ft2);
    //   printf("ft3: 0x%p, ", _caller_regs.ft3);
    //   printf("ft4: 0x%p\n", _caller_regs.ft4);
    //   printf("ft5: 0x%p, ", _caller_regs.ft5);
    //   printf("ft6: 0x%p, ", _caller_regs.ft6);
    //   printf("ft7: 0x%p, ", _caller_regs.ft7);
    //   printf("fa0: 0x%p\n", _caller_regs.fa0);
    //   printf("fa1: 0x%p", _caller_regs.fa1);
    //   printf("fa2: 0x%p", _caller_regs.fa2);
    //   printf("fa3: 0x%p", _caller_regs.fa3);
    //   printf("fa4: 0x%p", _caller_regs.fa4);
    //   printf("fa5: 0x%p", _caller_regs.fa5);
    //   printf("fa6: 0x%p", _caller_regs.fa6);
    //   printf("fa7: 0x%p", _caller_regs.fa7);
    //   printf("ft8: 0x%p", _caller_regs.ft8);
    //   printf("ft9: 0x%p", _caller_regs.ft9);
    //   printf("ft10: 0x%p", _caller_regs.ft10);
    //   printf("ft11: 0x%p", _caller_regs.ft11);
    //   return _os;
    // }
  };

  /**
   * @brief 被调用者保存寄存器
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
    // friend std::ostream &operator<<(std::ostream &_os,
    //                                 const CalleeRegs &_callee_regs) {
    //   printf("sp: 0x%p, ", _callee_regs.sp);
    //   printf("s0: 0x%p, ", _callee_regs.s0);
    //   printf("s1: 0x%p, ", _callee_regs.s1);
    //   printf("s2: 0x%p\n", _callee_regs.s2);
    //   printf("s3: 0x%p, ", _callee_regs.s3);
    //   printf("s4: 0x%p, ", _callee_regs.s4);
    //   printf("s5: 0x%p, ", _callee_regs.s5);
    //   printf("s6: 0x%p\n", _callee_regs.s6);
    //   printf("s7: 0x%p, ", _callee_regs.s7);
    //   printf("s8: 0x%p, ", _callee_regs.s8);
    //   printf("s9: 0x%p, ", _callee_regs.s9);
    //   printf("s10: 0x%p\n", _callee_regs.s10);
    //   printf("s11: 0x%p, ", _callee_regs.s11);
    //   printf("fs0: 0x%p, ", _callee_regs.fs0);
    //   printf("fs1: 0x%p, ", _callee_regs.fs1);
    //   printf("fs2: 0x%p\n", _callee_regs.fs2);
    //   printf("fs3: 0x%p, ", _callee_regs.fs3);
    //   printf("fs4: 0x%p, ", _callee_regs.fs4);
    //   printf("fs5: 0x%p, ", _callee_regs.fs5);
    //   printf("fs6: 0x%p\n", _callee_regs.fs6);
    //   printf("fs7: 0x%p, ", _callee_regs.fs7);
    //   printf("fs8: 0x%p, ", _callee_regs.fs8);
    //   printf("fs9: 0x%p, ", _callee_regs.fs9);
    //   printf("fs10: 0x%p\n", _callee_regs.fs10);
    //   printf("fs11: 0x%p", _callee_regs.fs11);
    //   return _os;
    // }
  };

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
  static inline Xstatus ReadSstatus(void) {
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
  static inline Xip ReadSip(void) {
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
    __asm__ volatile("csrr %0, sscratch" : "=r"(x));
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
   * @return uint64_t         读到的值
   */
  static inline uint64_t ReadScause() {
    uint64_t x;
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
    stvec.xtvec_.base = addr;
    stvec.xtvec_.mode = Xtvec::kDirect;
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

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_CPU_HPP_

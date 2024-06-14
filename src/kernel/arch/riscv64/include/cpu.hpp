
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
 */
namespace cpu {

// 第一部分：寄存器定义
namespace reginfo {

struct RegInfoBase {
  using DataType = uint64_t;
  static constexpr uint64_t kBitOffset = 0;
  static constexpr uint64_t kBitWidth = 64;
  static constexpr uint64_t kBitMask = ~0;
  static constexpr uint64_t kAllSetMask = ~0;
};

/// 通用寄存器
struct FpInfo : public RegInfoBase {};

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
    if constexpr (std::is_same<Reg, reginfo::FpInfo>::value) {
      __asm__ volatile("mv %0, fp" : "=r"(value) : :);
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
    if constexpr (std::is_same<Reg, reginfo::FpInfo>::value) {
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

// 第三部分：寄存器实例
class Fp : public ReadWriteRegBase<reginfo::FpInfo> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const Fp &fp) {
    printf("val: 0x%p", fp.Read());
    return os;
  }
};

/// 通用寄存器
struct AllXreg {
  Fp fp;
};

};  // namespace

// 第四部分：访问接口
[[maybe_unused]] static AllXreg kAllXreg;

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
  uint64_t sp;
  uint64_t s0;
  uint64_t s1;
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
  uint64_t fs0;
  uint64_t fs1;
  uint64_t fs2;
  uint64_t fs3;
  uint64_t fs4;
  uint64_t fs5;
  uint64_t fs6;
  uint64_t fs7;
  uint64_t fs8;
  uint64_t fs9;
  uint64_t fs10;
  uint64_t fs11;
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
  uint64_t xregs;
  uint64_t fregs;
  uint64_t sepc;
  uint64_t stval;
  uint64_t xcause;
  uint64_t xie;
  uint64_t xstatus;
  uint64_t satp;
  uint64_t sscratch;
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
 * @brief 上下文，用于任务切换
 * @note caller_regs 由编译器保存/恢复
 */
struct Context {
  /// 运行此任务的 core id
  uint64_t coreid;
  uint64_t ra;
  CalleeRegs callee_regs;
  uint64_t satp;
  uint64_t sepc;
  uint64_t sstatus;
  uint64_t sie;
  uint64_t sip;
  uint64_t sscratch;
  friend std::ostream &operator<<(std::ostream &os, const Context &context) {
    printf("coreid: 0x%X, ", context.coreid);
    printf("ra: 0x%p\n", context.ra);
    std::cout << context.callee_regs << std::endl;
    printf("satp: 0x%p, ", context.satp);
    printf("sepc: 0x%p, ", context.sepc);
    printf("xstatus: 0x%p, ", context.sstatus);
    printf("sie: 0x%p, ", context.sie);
    printf("sip: 0x%p, ", context.sip);
    printf("sscratch: 0x%p", context.sscratch);
    return os;
  }
};
};  // namespace cpu

namespace cpu::csr {

static AllCsr kAllCsr;

};

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_

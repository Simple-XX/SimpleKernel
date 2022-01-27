
/**
 * @file cpu.hpp
 * @brief cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef _CPU_HPP_
#define _CPU_HPP_

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "iostream"

/**
 * @brief cpu 相关
 * @todo
 */
namespace CPU {
// Supervisor Status Register, sstatus
// User Interrupt Enable
static constexpr const uint64_t SSTATUS_UIE = 1 << 0;
// Supervisor Interrupt Enable
static constexpr const uint64_t SSTATUS_SIE = 1 << 1;
// User Previous Interrupt Enable
static constexpr const uint64_t SSTATUS_UPIE = 1 << 4;
// Supervisor Previous Interrupt Enable
static constexpr const uint64_t SSTATUS_SPIE = 1 << 5;
// Previous mode, 1=Supervisor, 0=User
static constexpr const uint64_t SSTATUS_SPP = 1 << 8;

/**
 * @brief 读取 sstatus 寄存器
 * @return uint64_t         读取到的值
 */
static inline uint64_t READ_SSTATUS(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, sstatus" : "=r"(x));
    return x;
}

/**
 * @brief 写 sstatus 寄存器
 * @param  _x                要写的值
 */
static inline void WRITE_SSTATUS(uint64_t _x) {
    __asm__ volatile("csrw sstatus, %0" : : "r"(_x));
}

/**
 * @brief 读 sip
 * @return uint64_t         读取到的值
 * @note Supervisor Interrupt Pending
 */
static inline uint64_t READ_SIP(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, sip" : "=r"(x));
    return x;
}

/**
 * @brief 写 sip
 * @param  _x               要写的值
 */
static inline void WRITE_SIP(uint64_t _x) {
    __asm__ volatile("csrw sip, %0" : : "r"(_x));
    return;
}

// Supervisor Interrupt Enable
// software
static constexpr const uint64_t SIE_SSIE = 1 << 1;
// timer
static constexpr const uint64_t SIE_STIE = 1 << 5;
// external
static constexpr const uint64_t SIE_SEIE = 1 << 9;

/**
 * @brief 读 sie
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_SIE(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, sie" : "=r"(x));
    return x;
}

/**
 * @brief 写 sie
 * @param  _x                要写的值
 */
static inline void WRITE_SIE(uint64_t _x) {
    __asm__ volatile("csrw sie, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 sepc
 * @return uint64_t         读到的值
 * @note machine exception program counter, holds the instruction address to
 * which a return from exception will go.
 */
static inline uint64_t READ_SEPC(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, sepc" : "=r"(x));
    return x;
}

/**
 * @brief 写 sepc
 * @param  _x               要写的值
 */
static inline void WRITE_SEPC(uint64_t _x) {
    __asm__ volatile("csrw sepc, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 stvec
 * @return uint64_t         读到的值
 * @note Supervisor Trap-Vector Base Address low two bits are mode.
 */
static inline uint64_t READ_STVEC(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, stvec" : "=r"(x));
    return x;
}

/**
 * @brief 写 stvec
 * @param  _x               要写的值
 */
static inline void WRITE_STVEC(uint64_t _x) {
    __asm__ volatile("csrw stvec, %0" : : "r"(_x));
    return;
}

/// 中断模式 直接
static constexpr const uint64_t TVEC_DIRECT = 0xFFFFFFFFFFFFFFFC;
/// 中断模式 向量
static constexpr const uint64_t TVEC_VECTORED = 0xFFFFFFFFFFFFFFFD;

/**
 * @brief 设置中断模式，直接
 */
static inline void STVEC_DIRECT(void) {
    uint64_t stvec = READ_STVEC();
    stvec          = stvec & TVEC_DIRECT;
    WRITE_STVEC(stvec);
    return;
}

/**
 * @brief 设置中断模式，向量
 */
static inline void STVEC_VECTORED(void) {
    uint64_t stvec = READ_STVEC();
    stvec          = stvec & TVEC_VECTORED;
    WRITE_STVEC(stvec);
    return;
}

/// sv39 虚拟内存模式
static constexpr const uint64_t SATP_SV39 = (uint64_t)8 << 60;

/**
 * @brief 设置 sv39 虚拟内存模式
 * @param  _pgd             要设置的页目录
 * @return constexpr uintptr_t 设置好的页目录
 */
static constexpr uintptr_t SET_SV39(uintptr_t _pgd) {
    return (SATP_SV39 | (_pgd >> 12));
}

/**
 * @brief 设置页目录
 * @param  _x               要设置的页目录
 * @note supervisor address translation and protection; holds the address of
 * the page table.
 */
static inline void SET_PGD(uintptr_t _x) {
    uintptr_t old;
    // 读取现在的 pgd
    __asm__ volatile("csrr %0, satp" : "=r"(old));
    // 如果开启了 sv39
    if ((old & SATP_SV39) == SATP_SV39) {
        // 将新的页目录也设为开启
        _x = SET_SV39(_x);
    }
    __asm__ volatile("csrw satp, %0" : : "r"(_x));
    return;
}

/**
 * @brief 获取页目录
 * @return uintptr_t        页目录
 */
static inline uintptr_t GET_PGD(void) {
    uintptr_t x;
    __asm__ volatile("csrr %0, satp" : "=r"(x));
    // 如果开启了虚拟内存，恢复为原始格式
    if ((x & SATP_SV39) == SATP_SV39) {
        x = (x & 0x7FFFFFFFFF);
        x = (x << 12);
    }
    return x;
}

/**
 * @brief 开启分页
 * @return true             成功
 * @return false            失败
 */
static inline bool ENABLE_PG(void) {
    uintptr_t x = GET_PGD();
    SET_PGD(SET_SV39(x));
    info("paging enabled.\n");
    return true;
}

/**
 * @brief 读 sscratch 寄存器
 * @param  _x                要写的值
 */
static inline uint64_t READ_SSCRATCH(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, sscratch" : "=r"(x));
    return x;
}

/**
 * @brief 写 sscratch 寄存器
 * @param  _x                要写的值
 */
static inline void WRITE_SSCRATCH(uint64_t _x) {
    __asm__ volatile("csrw sscratch, %0" : : "r"(_x));
    return;
}

/// [31]=1 interrupt, else exception
static constexpr const uint64_t CAUSE_INTR_MASK = 0x8000000000000000;
/// low bits show code
static constexpr const uint64_t CAUSE_CODE_MASK = 0x7FFFFFFFFFFFFFFF;

/**
 * @brief 读 scause 寄存器 Supervisor Trap Cause
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_SCAUSE(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, scause" : "=r"(x));
    return x;
}

/**
 * @brief 读 stval 寄存器 Supervisor Trap Value
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_STVAL(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, stval" : "=r"(x));
    return x;
}

/**
 * @brief 读 time 寄存器 supervisor-mode cycle counter
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_TIME(void) {
    uint64_t x;
    // __asm__ volatile("csrr %0, time" : "=r" (x) );
    // this instruction will trap in SBI
    __asm__ volatile("rdtime %0" : "=r"(x));
    return x;
}

/**
 * @brief 允许中断
 */
static inline void ENABLE_INTR(void) {
    WRITE_SSTATUS(READ_SSTATUS() | SSTATUS_SIE);
    return;
}

/**
 * @brief 禁止中断
 */
static inline void DISABLE_INTR(void) {
    WRITE_SSTATUS(READ_SSTATUS() & ~SSTATUS_SIE);
    return;
}

/**
 * @brief 读取中断状态
 * @return true             允许
 * @return false            禁止
 */
static inline bool STATUS_INTR(void) {
    uint64_t x = READ_SSTATUS();
    return (x & SSTATUS_SIE) != 0;
}

/**
 * @brief 读 sp 寄存器
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_SP(void) {
    uint64_t x;
    __asm__ volatile("mv %0, sp" : "=r"(x));
    return x;
}

/**
 * @brief 读 tp 寄存器
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_TP(void) {
    uint64_t x;
    __asm__ volatile("mv %0, tp" : "=r"(x));
    return x;
}

/**
 * @brief 写 tp 寄存器
 * @param  _x                要写的值
 */
static inline void WRITE_TP(uint64_t _x) {
    __asm__ volatile("mv tp, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 ra 寄存器
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_RA(void) {
    uint64_t x;
    __asm__ volatile("mv %0, ra" : "=r"(x));
    return x;
}

/**
 * @brief 读 fp(s0) 寄存器
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_FP(void) {
    uint64_t fp;
    asm volatile("mv %0, s0" : "=r"(fp));
    return fp;
}

/**
 * @brief 刷新 tlb
 */
static inline void VMM_FLUSH(uintptr_t) {
    // the zero, zero means flush all TLB entries.
    __asm__ volatile("sfence.vma zero, zero");
    return;
}

/**
 * @brief 通用寄存器
 */
struct xregs_t {
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
};

/**
 * @brief 浮点寄存器
 */
struct fregs_t {
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
};

/**
 * @brief 调用者保存寄存器
 */
struct caller_regs_t {
    uintptr_t            ra;
    uintptr_t            t0;
    uintptr_t            t2;
    uintptr_t            a0;
    uintptr_t            a1;
    uintptr_t            a2;
    uintptr_t            a3;
    uintptr_t            a4;
    uintptr_t            a5;
    uintptr_t            a6;
    uintptr_t            a7;
    uintptr_t            t3;
    uintptr_t            t4;
    uintptr_t            t5;
    uintptr_t            t6;
    uintptr_t            ft0;
    uintptr_t            ft1;
    uintptr_t            ft2;
    uintptr_t            ft3;
    uintptr_t            ft4;
    uintptr_t            ft5;
    uintptr_t            ft6;
    uintptr_t            ft7;
    uintptr_t            fa0;
    uintptr_t            fa1;
    uintptr_t            fa2;
    uintptr_t            fa3;
    uintptr_t            fa4;
    uintptr_t            fa5;
    uintptr_t            fa6;
    uintptr_t            fa7;
    uintptr_t            ft8;
    uintptr_t            ft9;
    uintptr_t            ft10;
    uintptr_t            ft11;
    friend std::ostream &operator<<(std::ostream        &_os,
                                    const caller_regs_t &_caller_regs) {
        printf("ra: 0x%p, ", _caller_regs.ra);
        printf("t0: 0x%p, ", _caller_regs.t0);
        printf("t2: 0x%p, ", _caller_regs.t2);
        printf("a0: 0x%p\n", _caller_regs.a0);
        printf("a1: 0x%p, ", _caller_regs.a1);
        printf("a2: 0x%p, ", _caller_regs.a2);
        printf("a3: 0x%p, ", _caller_regs.a3);
        printf("a4: 0x%p\n", _caller_regs.a4);
        printf("a5: 0x%p, ", _caller_regs.a5);
        printf("a6: 0x%p, ", _caller_regs.a6);
        printf("a7: 0x%p, ", _caller_regs.a7);
        printf("t3: 0x%p\n", _caller_regs.t3);
        printf("t4: 0x%p, ", _caller_regs.t4);
        printf("t5: 0x%p, ", _caller_regs.t5);
        printf("t6: 0x%p, ", _caller_regs.t6);
        printf("ft0: 0x%p\n", _caller_regs.ft0);
        printf("ft1: 0x%p, ", _caller_regs.ft1);
        printf("ft2: 0x%p, ", _caller_regs.ft2);
        printf("ft3: 0x%p, ", _caller_regs.ft3);
        printf("ft4: 0x%p\n", _caller_regs.ft4);
        printf("ft5: 0x%p, ", _caller_regs.ft5);
        printf("ft6: 0x%p, ", _caller_regs.ft6);
        printf("ft7: 0x%p, ", _caller_regs.ft7);
        printf("fa0: 0x%p\n", _caller_regs.fa0);
        printf("fa1: 0x%p", _caller_regs.fa1);
        printf("fa2: 0x%p", _caller_regs.fa2);
        printf("fa3: 0x%p", _caller_regs.fa3);
        printf("fa4: 0x%p", _caller_regs.fa4);
        printf("fa5: 0x%p", _caller_regs.fa5);
        printf("fa6: 0x%p", _caller_regs.fa6);
        printf("fa7: 0x%p", _caller_regs.fa7);
        printf("ft8: 0x%p", _caller_regs.ft8);
        printf("ft9: 0x%p", _caller_regs.ft9);
        printf("ft10: 0x%p", _caller_regs.ft10);
        printf("ft11: 0x%p", _caller_regs.ft11);
        return _os;
    }
};

/**
 * @brief 被调用者保存寄存器
 */
struct callee_regs_t {
    uintptr_t            sp;
    uintptr_t            s0;
    uintptr_t            s1;
    uintptr_t            s2;
    uintptr_t            s3;
    uintptr_t            s4;
    uintptr_t            s5;
    uintptr_t            s6;
    uintptr_t            s7;
    uintptr_t            s8;
    uintptr_t            s9;
    uintptr_t            s10;
    uintptr_t            s11;
    uintptr_t            fs0;
    uintptr_t            fs1;
    uintptr_t            fs2;
    uintptr_t            fs3;
    uintptr_t            fs4;
    uintptr_t            fs5;
    uintptr_t            fs6;
    uintptr_t            fs7;
    uintptr_t            fs8;
    uintptr_t            fs9;
    uintptr_t            fs10;
    uintptr_t            fs11;
    friend std::ostream &operator<<(std::ostream        &_os,
                                    const callee_regs_t &_callee_regs) {
        printf("sp: 0x%p, ", _callee_regs.sp);
        printf("s0: 0x%p, ", _callee_regs.s0);
        printf("s1: 0x%p, ", _callee_regs.s1);
        printf("s2: 0x%p\n", _callee_regs.s2);
        printf("s3: 0x%p, ", _callee_regs.s3);
        printf("s4: 0x%p, ", _callee_regs.s4);
        printf("s5: 0x%p, ", _callee_regs.s5);
        printf("s6: 0x%p\n", _callee_regs.s6);
        printf("s7: 0x%p, ", _callee_regs.s7);
        printf("s8: 0x%p, ", _callee_regs.s8);
        printf("s9: 0x%p, ", _callee_regs.s9);
        printf("s10: 0x%p\n", _callee_regs.s10);
        printf("s11: 0x%p, ", _callee_regs.s11);
        printf("fs0: 0x%p, ", _callee_regs.fs0);
        printf("fs1: 0x%p, ", _callee_regs.fs1);
        printf("fs2: 0x%p\n", _callee_regs.fs2);
        printf("fs3: 0x%p, ", _callee_regs.fs3);
        printf("fs4: 0x%p, ", _callee_regs.fs4);
        printf("fs5: 0x%p, ", _callee_regs.fs5);
        printf("fs6: 0x%p\n", _callee_regs.fs6);
        printf("fs7: 0x%p, ", _callee_regs.fs7);
        printf("fs8: 0x%p, ", _callee_regs.fs8);
        printf("fs9: 0x%p, ", _callee_regs.fs9);
        printf("fs10: 0x%p\n", _callee_regs.fs10);
        printf("fs11: 0x%p", _callee_regs.fs11);
        return _os;
    }
};

/**
 * @brief 上下文，用于中断/任务切换
 */
struct context_t {
    uintptr_t ra;
    //    uintptr_t            tp;
    //    CPU::caller_regs_t   caller_regs;
    CPU::callee_regs_t   callee_regs;
    uintptr_t            satp;
    uintptr_t            sepc;
    uintptr_t            sstatus;
    uintptr_t            sie;
    uintptr_t            sip;
    uintptr_t            sscratch;
    friend std::ostream &operator<<(std::ostream    &_os,
                                    const context_t &_context) {
        printf("ra: 0x%p\n", _context.ra);
        //        std::cout << _context.caller_regs << std::endl;
        printf("satp: 0x%p, ", _context.satp);
        printf("sepc: 0x%p, ", _context.sepc);
        printf("sstatus: 0x%p, ", _context.sstatus);
        printf("sie: 0x%p, ", _context.sie);
        printf("sip: 0x%p, ", _context.sip);
        //        printf("tp: 0x%p, ", _context.tp);
        printf("sscratch: 0x%p", _context.sscratch);
        return _os;
    }
};

/**
 * @brief 设置当前 core id
 * @param  _hartid          要设置的值
 * @todo 不使用 tp
 */
static inline void set_curr_core_id(size_t _hartid) {
    CPU::WRITE_TP(_hartid);
    return;
}

/**
 * @brief 获取当前 core id
 * @return size_t           hartid
 * @note hartid 和 core id 是一回事
 * @todo 不使用 tp
 */
static inline size_t get_curr_core_id(void) {
    return CPU::READ_TP();
}

}; // namespace CPU

#endif /* _CPU_HPP_ */

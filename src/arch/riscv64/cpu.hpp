
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
 * @brief mstatus 寄存器定义
 */
struct mstatus_t {
    union {
        struct {
            // interrupt enable
            uint64_t ie : 4;
            // previous interrupt enable
            uint64_t pie : 4;
            // previous mode (supervisor)
            uint64_t spp : 1;
            uint64_t unused1 : 2;
            // previous mode (machine)
            uint64_t mpp : 2;
            // FPU status
            uint64_t fs : 2;
            // extensions status
            uint64_t xs : 2;
            // modify privilege
            uint64_t mprv : 1;
            // permit supervisor user memory access
            uint64_t sum : 1;
            // make executable readable
            uint64_t mxr : 1;
            // trap virtual memory
            uint64_t tvm : 1;
            // timeout wait (trap WFI)
            uint64_t tw : 1;
            // trap SRET
            uint64_t tsr : 1;
            uint64_t unused2 : 9;
            // U-mode XLEN
            uint64_t uxl : 2;
            // S-mode XLEN
            uint64_t sxl : 2;
            uint64_t unused3 : 27;
            // status dirty
            uint64_t sd : 1;
        };
        uint64_t val;
    };

    mstatus_t(void) {
        return;
    }
    mstatus_t(uint64_t _val) : val(_val) {
        return;
    }
};

/**
 * @brief sstatus 寄存器定义
 */
struct sstatus_t {
    union {
        struct {
            // interrupt enable
            uint64_t ie : 2;
            uint64_t unused1 : 2;
            // previous interrupt enable
            uint64_t pie : 2;
            uint64_t unused2 : 2;
            // previous mode (supervisor)
            uint64_t spp : 1;
            uint64_t unused3 : 4;
            // FPU status
            uint64_t fs : 2;
            // extensions status
            uint64_t xs : 2;
            uint64_t unused4 : 1;
            // permit supervisor user memory access
            uint64_t sum : 1;
            // make executable readable
            uint64_t mxr : 1;
            uint64_t unused5 : 12;
            // U-mode XLEN
            uint64_t uxl : 2;
            uint64_t unused6 : 29;
            // status dirty
            uint64_t sd : 1;
        };
        uint64_t val;
    };

    sstatus_t(void) {
        return;
    }
    sstatus_t(uint64_t _val) : val(_val) {
        return;
    }
};

/**
 * @brief 读取 sstatus 寄存器
 * @return uint64_t         读取到的值
 */
static inline uint64_t READ_SSTATUS(void) {
    uint64_t x;
    asm("csrr %0, sstatus" : "=r"(x));
    return x;
}

/**
 * @brief 写 sstatus 寄存器
 * @param  _x                要写的值
 */
static inline void WRITE_SSTATUS(uint64_t _x) {
    asm("csrw sstatus, %0" : : "r"(_x));
}

/**
 * @brief 读 sip
 * @return uint64_t         读取到的值
 * @note Supervisor Interrupt Pending
 */
static inline uint64_t READ_SIP(void) {
    uint64_t x;
    asm("csrr %0, sip" : "=r"(x));
    return x;
}

/**
 * @brief 写 sip
 * @param  _x               要写的值
 */
static inline void WRITE_SIP(uint64_t _x) {
    asm("csrw sip, %0" : : "r"(_x));
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
    asm("csrr %0, sie" : "=r"(x));
    return x;
}

/**
 * @brief 写 sie
 * @param  _x                要写的值
 */
static inline void WRITE_SIE(uint64_t _x) {
    asm("csrw sie, %0" : : "r"(_x));
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
    asm("csrr %0, sepc" : "=r"(x));
    return x;
}

/**
 * @brief 写 sepc
 * @param  _x               要写的值
 */
static inline void WRITE_SEPC(uint64_t _x) {
    asm("csrw sepc, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 stvec
 * @return uint64_t         读到的值
 * @note Supervisor Trap-Vector Base Address low two bits are mode.
 */
static inline uint64_t READ_STVEC(void) {
    uint64_t x;
    asm("csrr %0, stvec" : "=r"(x));
    return x;
}

/**
 * @brief 写 stvec
 * @param  _x               要写的值
 */
static inline void WRITE_STVEC(uint64_t _x) {
    asm("csrw stvec, %0" : : "r"(_x));
    return;
}

/**
 * @brief satp 结构
 */
struct satp_t {
    enum {
        NONE = 0,
        SV39 = 8,
        SV48 = 9,
        SV57 = 10,
        SV64 = 11,
    };

    union {
        struct {
            uint64_t ppn : 44;
            uint64_t asid : 16;
            uint64_t mode : 4;
        };
        uint64_t val;
    };

    satp_t(void) {
        return;
    }
    satp_t(uint64_t _val) : val(_val) {
        return;
    }
};

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

/**
 * @brief 设置 sv39 虚拟内存模式
 * @param  _pgd             要设置的页目录
 * @return constexpr uintptr_t 设置好的页目录
 */
static uintptr_t SET_SV39(uintptr_t _pgd) {
    satp_t satp;
    satp.val  = _pgd >> 12;
    satp.asid = 0;
    satp.mode = satp_t::SV39;
    return satp.val;
}

/**
 * @brief 设置页目录
 * @param  _x               要设置的页目录
 * @note supervisor address translation and protection; holds the address of
 * the page table.
 * @todo 需要判断 _x 是否已经处理过
 */
static inline void SET_PGD(uintptr_t _x) {
    satp_t satp_old;
    satp_t satp_new;
    satp_new.val  = _x;
    satp_new.asid = 0;
    // 读取现在的 pgd
    asm("csrr %0, satp" : "=r"(satp_old));
    // 如果开启了 sv39
    if (satp_old.mode == satp_t::SV39) {
        // 将新的页目录也设为开启
        satp_new.mode = satp_t::SV39;
    }
    asm("csrw satp, %0" : : "r"(satp_new));
    return;
}

/**
 * @brief 获取页目录
 * @return uintptr_t        页目录
 */
static inline uintptr_t GET_PGD(void) {
    satp_t satp;
    asm("csrr %0, satp" : "=r"(satp));
    // 如果开启了虚拟内存，恢复为原始格式
    if (satp.mode == satp_t::SV39) {
        return satp.ppn << 12;
    }
    return satp.val;
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
    asm("csrw sscratch, %0" : : "r"(_x));
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
    asm("csrr %0, scause" : "=r"(x));
    return x;
}

/**
 * @brief 读 stval 寄存器 Supervisor Trap Value
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_STVAL(void) {
    uint64_t x;
    asm("csrr %0, stval" : "=r"(x));
    return x;
}

/**
 * @brief 读 time 寄存器 supervisor-mode cycle counter
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_TIME(void) {
    uint64_t x;
    // asm ("csrr %0, time" : "=r" (x) );
    // this instruction will trap in SBI
    asm("rdtime %0" : "=r"(x));
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
 * @brief 允许中断
 * @param  _sstatus         要设置的 sstatus
 */
static inline void ENABLE_INTR(uint64_t &_sstatus) {
    _sstatus |= SSTATUS_SIE;
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
 * @brief 禁止中断
 * @param  _sstatus         要设置的原 sstatus 值
 */
static inline void DISABLE_INTR(uint64_t &_sstatus) {
    _sstatus &= ~SSTATUS_SIE;
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
    asm("mv %0, sp" : "=r"(x));
    return x;
}

/**
 * @brief 读 tp 寄存器
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_TP(void) {
    uint64_t x;
    asm("mv %0, tp" : "=r"(x));
    return x;
}

/**
 * @brief 写 tp 寄存器
 * @param  _x                要写的值
 */
static inline void WRITE_TP(uint64_t _x) {
    asm("mv tp, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 ra 寄存器
 * @return uint64_t         读到的值
 */
static inline uint64_t READ_RA(void) {
    uint64_t x;
    asm("mv %0, ra" : "=r"(x));
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
    asm("sfence.vma zero, zero");
    return;
}

/**
 * @brief 通用寄存器
 */
struct xregs_t {
    uintptr_t            zero;
    uintptr_t            ra;
    uintptr_t            sp;
    uintptr_t            gp;
    uintptr_t            tp;
    uintptr_t            t0;
    uintptr_t            t1;
    uintptr_t            t2;
    uintptr_t            s0;
    uintptr_t            s1;
    uintptr_t            a0;
    uintptr_t            a1;
    uintptr_t            a2;
    uintptr_t            a3;
    uintptr_t            a4;
    uintptr_t            a5;
    uintptr_t            a6;
    uintptr_t            a7;
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
    uintptr_t            t3;
    uintptr_t            t4;
    uintptr_t            t5;
    uintptr_t            t6;
    friend std::ostream &operator<<(std::ostream &_os, const xregs_t &_xregs) {
        printf("zero: 0x%p, ", _xregs.zero);
        printf("ra: 0x%p, ", _xregs.ra);
        printf("sp: 0x%p, ", _xregs.sp);
        printf("gp: 0x%p\n", _xregs.gp);
        printf("tp: 0x%p, ", _xregs.tp);
        printf("t0: 0x%p, ", _xregs.t0);
        printf("t1: 0x%p, ", _xregs.t1);
        printf("t2: 0x%p\n", _xregs.t2);
        printf("s0: 0x%p, ", _xregs.s0);
        printf("s1: 0x%p, ", _xregs.s1);
        printf("a0: 0x%p, ", _xregs.a0);
        printf("a1: 0x%p\n", _xregs.a1);
        printf("a2: 0x%p, ", _xregs.a2);
        printf("a3: 0x%p, ", _xregs.a3);
        printf("a4: 0x%p, ", _xregs.a4);
        printf("a5: 0x%p\n", _xregs.a5);
        printf("a6: 0x%p, ", _xregs.a6);
        printf("a7: 0x%p, ", _xregs.a7);
        printf("s2: 0x%p, ", _xregs.s2);
        printf("s3: 0x%p\n", _xregs.s3);
        printf("s4: 0x%p, ", _xregs.s4);
        printf("s5: 0x%p, ", _xregs.s5);
        printf("s6: 0x%p, ", _xregs.s6);
        printf("s7: 0x%p\n", _xregs.s7);
        printf("s8: 0x%p, ", _xregs.s8);
        printf("s9: 0x%p, ", _xregs.s9);
        printf("s10: 0x%p, ", _xregs.s10);
        printf("s11: 0x%p\n", _xregs.s11);
        printf("t3: 0x%p, ", _xregs.t3);
        printf("t4: 0x%p, ", _xregs.t4);
        printf("t5: 0x%p, ", _xregs.t5);
        printf("t6: 0x%p", _xregs.t6);
        return _os;
    }
};

/**
 * @brief 浮点寄存器
 */
struct fregs_t {
    uintptr_t            ft0;
    uintptr_t            ft1;
    uintptr_t            ft2;
    uintptr_t            ft3;
    uintptr_t            ft4;
    uintptr_t            ft5;
    uintptr_t            ft6;
    uintptr_t            ft7;
    uintptr_t            fs0;
    uintptr_t            fs1;
    uintptr_t            fa0;
    uintptr_t            fa1;
    uintptr_t            fa2;
    uintptr_t            fa3;
    uintptr_t            fa4;
    uintptr_t            fa5;
    uintptr_t            fa6;
    uintptr_t            fa7;
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
    uintptr_t            ft8;
    uintptr_t            ft9;
    uintptr_t            ft10;
    uintptr_t            ft11;
    friend std::ostream &operator<<(std::ostream &_os, const fregs_t &_fregs) {
        printf("ft0: 0x%p, ", _fregs.ft0);
        printf("ft1: 0x%p, ", _fregs.ft1);
        printf("ft2: 0x%p, ", _fregs.ft2);
        printf("ft3: 0x%p\n", _fregs.ft3);
        printf("ft4: 0x%p, ", _fregs.ft4);
        printf("ft5: 0x%p, ", _fregs.ft5);
        printf("ft6: 0x%p, ", _fregs.ft6);
        printf("ft7: 0x%p\n", _fregs.ft7);
        printf("fs0: 0x%p, ", _fregs.fs0);
        printf("fs1: 0x%p, ", _fregs.fs1);
        printf("fa0: 0x%p, ", _fregs.fa0);
        printf("fa1: 0x%p\n", _fregs.fa1);
        printf("fa2: 0x%p, ", _fregs.fa2);
        printf("fa3: 0x%p, ", _fregs.fa3);
        printf("fa4: 0x%p, ", _fregs.fa4);
        printf("fa5: 0x%p\n", _fregs.fa5);
        printf("fa6: 0x%p, ", _fregs.fa6);
        printf("fa7: 0x%p, ", _fregs.fa7);
        printf("fs2: 0x%p, ", _fregs.fs2);
        printf("fs3: 0x%p\n", _fregs.fs3);
        printf("fs4: 0x%p, ", _fregs.fs4);
        printf("fs5: 0x%p, ", _fregs.fs5);
        printf("fs6: 0x%p, ", _fregs.fs6);
        printf("fs7: 0x%p\n", _fregs.fs7);
        printf("fs8: 0x%p, ", _fregs.fs8);
        printf("fs9: 0x%p, ", _fregs.fs9);
        printf("fs10: 0x%p, ", _fregs.fs10);
        printf("fs11: 0x%p\n", _fregs.fs11);
        printf("ft8: 0x%p, ", _fregs.ft8);
        printf("ft9: 0x%p, ", _fregs.ft9);
        printf("ft10: 0x%p, ", _fregs.ft10);
        printf("ft11: 0x%p", _fregs.ft11);
        return _os;
    }
};

/**
 * @brief 所有寄存器，在中断时使用，共 32+32+6=70 个
 */
struct all_regs_t {
    xregs_t              xregs;
    fregs_t              fregs;
    uintptr_t            sepc;
    uintptr_t            stval;
    uintptr_t            scause;
    uintptr_t            sie;
    uintptr_t            sstatus;
    uintptr_t            sscratch;
    friend std::ostream &operator<<(std::ostream     &_os,
                                    const all_regs_t &_all_regs) {
        (void)_all_regs.fregs;
        _os << _all_regs.xregs << std::endl;
        printf(
            "sepc: 0x%p, stval: 0x%p, scause: 0x%p, sie: 0x%p, sstatus: 0x%p, "
            "sscratch: 0x%p",
            _all_regs.sepc, _all_regs.stval, _all_regs.scause, _all_regs.sie,
            _all_regs.sstatus, _all_regs.sscratch);
        return _os;
    }
};

/**
 * @brief 读取所有寄存器
 * @param  _all_regs        要保存读取到的的值
 */
static inline void read_all_reg(all_regs_t &_all_regs) {
    asm("mv %0, zero" : "=r"(_all_regs.xregs.zero));
    asm("mv %0, ra" : "=r"(_all_regs.xregs.ra));
    asm("mv %0, sp" : "=r"(_all_regs.xregs.sp));
    asm("mv %0, gp" : "=r"(_all_regs.xregs.gp));
    asm("mv %0, tp" : "=r"(_all_regs.xregs.tp));
    asm("mv %0, t0" : "=r"(_all_regs.xregs.t0));
    asm("mv %0, t1" : "=r"(_all_regs.xregs.t1));
    asm("mv %0, t2" : "=r"(_all_regs.xregs.t2));
    asm("mv %0, s0" : "=r"(_all_regs.xregs.s0));
    asm("mv %0, s1" : "=r"(_all_regs.xregs.s1));
    asm("mv %0, a0" : "=r"(_all_regs.xregs.a0));
    asm("mv %0, a1" : "=r"(_all_regs.xregs.a1));
    asm("mv %0, a2" : "=r"(_all_regs.xregs.a2));
    asm("mv %0, a3" : "=r"(_all_regs.xregs.a3));
    asm("mv %0, a4" : "=r"(_all_regs.xregs.a4));
    asm("mv %0, a5" : "=r"(_all_regs.xregs.a5));
    asm("mv %0, a6" : "=r"(_all_regs.xregs.a6));
    asm("mv %0, a7" : "=r"(_all_regs.xregs.a7));
    asm("mv %0, s2" : "=r"(_all_regs.xregs.s2));
    asm("mv %0, s3" : "=r"(_all_regs.xregs.s3));
    asm("mv %0, s4" : "=r"(_all_regs.xregs.s4));
    asm("mv %0, s5" : "=r"(_all_regs.xregs.s5));
    asm("mv %0, s6" : "=r"(_all_regs.xregs.s6));
    asm("mv %0, s7" : "=r"(_all_regs.xregs.s7));
    asm("mv %0, s8" : "=r"(_all_regs.xregs.s8));
    asm("mv %0, s9" : "=r"(_all_regs.xregs.s9));
    asm("mv %0, s10" : "=r"(_all_regs.xregs.s10));
    asm("mv %0, s11" : "=r"(_all_regs.xregs.s11));
    asm("mv %0, t3" : "=r"(_all_regs.xregs.t3));
    asm("mv %0, t4" : "=r"(_all_regs.xregs.t4));
    asm("mv %0, t5" : "=r"(_all_regs.xregs.t5));
    asm("mv %0, t6" : "=r"(_all_regs.xregs.t6));

    //    asm("mv %0, ft0" : "=r"(_all_regs.fregs.ft0));
    //    asm("mv %0, ft1" : "=r"(_all_regs.fregs.ft1));
    //    asm("mv %0, ft2" : "=r"(_all_regs.fregs.ft2));
    //    asm("mv %0, ft3" : "=r"(_all_regs.fregs.ft3));
    //    asm("mv %0, ft4" : "=r"(_all_regs.fregs.ft4));
    //    asm("mv %0, ft5" : "=r"(_all_regs.fregs.ft5));
    //    asm("mv %0, ft6" : "=r"(_all_regs.fregs.ft6));
    //    asm("mv %0, ft7" : "=r"(_all_regs.fregs.ft7));
    //    asm("mv %0, fs0" : "=r"(_all_regs.fregs.fs0));
    //    asm("mv %0, fs1" : "=r"(_all_regs.fregs.fs1));
    //    asm("mv %0, fa0" : "=r"(_all_regs.fregs.fa0));
    //    asm("mv %0, fa1" : "=r"(_all_regs.fregs.fa1));
    //    asm("mv %0, fa2" : "=r"(_all_regs.fregs.fa2));
    //    asm("mv %0, fa3" : "=r"(_all_regs.fregs.fa3));
    //    asm("mv %0, fa4" : "=r"(_all_regs.fregs.fa4));
    //    asm("mv %0, fa5" : "=r"(_all_regs.fregs.fa5));
    //    asm("mv %0, fa6" : "=r"(_all_regs.fregs.fa6));
    //    asm("mv %0, fa7" : "=r"(_all_regs.fregs.fa7));
    //    asm("mv %0, fs2" : "=r"(_all_regs.fregs.fs2));
    //    asm("mv %0, fs3" : "=r"(_all_regs.fregs.fs3));
    //    asm("mv %0, fs4" : "=r"(_all_regs.fregs.fs4));
    //    asm("mv %0, fs5" : "=r"(_all_regs.fregs.fs5));
    //    asm("mv %0, fs6" : "=r"(_all_regs.fregs.fs6));
    //    asm("mv %0, fs7" : "=r"(_all_regs.fregs.fs7));
    //    asm("mv %0, fs8" : "=r"(_all_regs.fregs.fs8));
    //    asm("mv %0, fs9" : "=r"(_all_regs.fregs.fs9));
    //    asm("mv %0, fs10" : "=r"(_all_regs.fregs.fs10));
    //    asm("mv %0, fs11" : "=r"(_all_regs.fregs.fs11));
    //    asm("mv %0, ft8" : "=r"(_all_regs.fregs.ft8));
    //    asm("mv %0, ft9" : "=r"(_all_regs.fregs.ft9));
    //    asm("mv %0, ft10" : "=r"(_all_regs.fregs.ft10));
    //    asm("mv %0, ft11" : "=r"(_all_regs.fregs.ft11));

    asm volatile("csrr %0, sepc" : "=r"(_all_regs.sepc));
    asm volatile("csrr %0, stval" : "=r"(_all_regs.stval));
    asm volatile("csrr %0, scause" : "=r"(_all_regs.scause));
    asm volatile("csrr %0, sstatus" : "=r"(_all_regs.sstatus));
    asm volatile("csrr %0, sscratch" : "=r"(_all_regs.sscratch));

    return;
}

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
 * @brief 上下文，用于任务切换
 * @note caller_regs 由编译器保存/恢复
 */
struct context_t {
    /// 运行此任务的 core id
    uintptr_t            coreid;
    uintptr_t            ra;
    CPU::callee_regs_t   callee_regs;
    uintptr_t            satp;
    uintptr_t            sepc;
    uintptr_t            sstatus;
    uintptr_t            sie;
    uintptr_t            sip;
    uintptr_t            sscratch;
    friend std::ostream &operator<<(std::ostream    &_os,
                                    const context_t &_context) {
        printf("coreid: 0x%X, ", _context.coreid);
        printf("ra: 0x%p\n", _context.ra);
        std::cout << _context.callee_regs << std::endl;
        printf("satp: 0x%p, ", _context.satp);
        printf("sepc: 0x%p, ", _context.sepc);
        printf("sstatus: 0x%p, ", _context.sstatus);
        printf("sie: 0x%p, ", _context.sie);
        printf("sip: 0x%p, ", _context.sip);
        printf("sscratch: 0x%p", _context.sscratch);
        return _os;
    }
};

}; // namespace CPU

#endif /* _CPU_HPP_ */

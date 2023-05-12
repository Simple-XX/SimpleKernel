
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

#ifndef SIMPLEKERNEL_CPU_HPP
#define SIMPLEKERNEL_CPU_HPP

#include "cstdbool"
#include "cstdint"
#include "cstdio"
#include "iostream"

/**
 * @brief cpu 相关
 * @todo
 */
namespace CPU {
/**
 * @brief pte 结构
 * @todo 使用 pte 结构重写 vmm
 */
struct pte_t {
    enum {
        VALID_OFFSET    = 0,
        READ_OFFSET     = 1,
        WRITE_OFFSET    = 2,
        EXEC_OFFSET     = 3,
        USER_OFFSET     = 4,
        GLOBAL_OFFSET   = 5,
        ACCESSED_OFFSET = 6,
        DIRTY_OFFSET    = 7,
        VALID           = 1 << VALID_OFFSET,
        READ            = 1 << READ_OFFSET,
        WRITE           = 1 << WRITE_OFFSET,
        EXEC            = 1 << EXEC_OFFSET,
        USER            = 1 << USER_OFFSET,
        GLOBAL          = 1 << GLOBAL_OFFSET,
        ACCESSED        = 1 << ACCESSED_OFFSET,
        DIRTY           = 1 << DIRTY_OFFSET,
    };

    union {
        struct {
            uint64_t flags    : 8;
            uint64_t rsw      : 2;
            uint64_t ppn      : 44;
            uint64_t reserved : 10;
        };

        uint64_t val;
    };

    pte_t(void) {
        val = 0;
        return;
    }

    pte_t(uint64_t _val) : val(_val) {
        return;
    }

    friend std::ostream& operator<<(std::ostream& _os, const pte_t& _pte) {
        printf("val: 0x%p, valid: %s, read: %s, write: %s, exec: %s, user: %s, "
               "global: %s, accessed: %s, dirty: %s, rsw: 0x%p, ppn: 0x%p",
               _pte.val, (_pte.flags & VALID) == VALID ? "true" : "false",
               (_pte.flags & READ) == READ ? "true" : "false",
               (_pte.flags & WRITE) == WRITE ? "true" : "false",
               (_pte.flags & EXEC) == EXEC ? "true" : "false",
               (_pte.flags & USER) == USER ? "true" : "false",
               (_pte.flags & GLOBAL) == GLOBAL ? "true" : "false",
               (_pte.flags & ACCESSED) == ACCESSED ? "true" : "false",
               (_pte.flags & DIRTY) == DIRTY ? "true" : "false", _pte.rsw,
               _pte.ppn);
        return _os;
    }
};

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

    static constexpr const char* MODE_NAME[] = {
        [NONE] = "NONE", "UNKNOWN",       "UNKNOWN",       "UNKNOWN",
        "UNKNOWN",       "UNKNOWN",       "UNKNOWN",       "UNKNOWN",
        [SV39] = "SV39", [SV48] = "SV48", [SV57] = "SV57", [SV64] = "SV64",
    };

    union {
        struct {
            uint64_t ppn  : 44;
            uint64_t asid : 16;
            uint64_t mode : 4;
        };

        uint64_t val;
    };

    static constexpr const uint64_t PPN_OFFSET = 12;

    satp_t(void) {
        val = 0;
        return;
    }

    satp_t(uint64_t _val) : val(_val) {
        return;
    }

    friend std::ostream& operator<<(std::ostream& _os, const satp_t& _satp) {
        printf("val: 0x%p, ppn: 0x%p, asid: 0x%p, mode: %s", _satp.val,
               _satp.ppn, _satp.asid, MODE_NAME[_satp.mode]);
        return _os;
    }
};

/// 机器模式定义
enum {
    U_MODE = 0,
    S_MODE = 1,
    M_MODE = 3,
};

enum {
    INTR_SOFT     = 0,
    /// U 态软中断
    INTR_SOFT_U   = INTR_SOFT + U_MODE,
    /// S 态软中断
    INTR_SOFT_S   = INTR_SOFT + S_MODE,
    /// M 态软中断
    INTR_SOFT_M   = INTR_SOFT + M_MODE,
    INTR_TIMER    = 4,
    /// U 态时钟中断
    INTR_TIMER_U  = INTR_TIMER + U_MODE,
    /// S 态时钟中断
    INTR_TIMER_S  = INTR_TIMER + S_MODE,
    /// M 态时钟中断
    INTR_TIMER_M  = INTR_TIMER + M_MODE,
    INTR_EXTERN   = 8,
    /// U 态外部中断
    INTR_EXTERN_U = INTR_EXTERN + U_MODE,
    /// S 态外部中断
    INTR_EXTERN_S = INTR_EXTERN + S_MODE,
    /// M 态外部中断
    INTR_EXTERN_M = INTR_EXTERN + M_MODE,
};

enum {
    EXCP_INSTRUCTION_ADDRESS_MISALIGNED = 0,
    EXCP_INSTRUCTION_ACCESS_FAULT       = 1,
    EXCP_ILLEGAL_INSTRUCTION            = 2,
    EXCP_BREAKPOINT                     = 3,
    EXCP_LOAD_ADDRESS_MISALIGNED        = 4,
    EXCP_LOAD_ACCESS_FAULT              = 5,
    EXCP_STORE_AMO_ADDRESS_MISALIGNED   = 6,
    EXCP_STORE_AMO_ACCESS_FAULT         = 7,
    EXCP_ECALL                          = 8,
    EXCP_ECALL_U                        = EXCP_ECALL + U_MODE,
    EXCP_ECALL_S                        = EXCP_ECALL + S_MODE,
    EXCP_ECALL_M                        = EXCP_ECALL + M_MODE,
    EXCP_INSTRUCTION_PAGE_FAULT         = 12,
    EXCP_LOAD_PAGE_FAULT                = 13,
    EXCP_STORE_AMO_PAGE_FAULT           = 15,
};

// Supervisor Status Register, sstatus
// User Interrupt Enable
static constexpr const uint64_t SSTATUS_UIE  = 1 << 0;
// Supervisor Interrupt Enable
static constexpr const uint64_t SSTATUS_SIE  = 1 << 1;
// User Previous Interrupt Enable
static constexpr const uint64_t SSTATUS_UPIE = 1 << 4;
// Supervisor Previous Interrupt Enable
static constexpr const uint64_t SSTATUS_SPIE = 1 << 5;
// Previous mode, 1=Supervisor, 0=User
static constexpr const uint64_t SSTATUS_SPP  = 1 << 8;

/**
 * @brief mstatus 寄存器定义
 */
struct mstatus_t {
    union {
        struct {
            // interrupt enable
            uint64_t ie      : 4;
            // previous interrupt enable
            uint64_t pie     : 4;
            // previous mode (supervisor)
            uint64_t spp     : 1;
            uint64_t unused1 : 2;
            // previous mode (machine)
            uint64_t mpp     : 2;
            // FPU status
            uint64_t fs      : 2;
            // extensions status
            uint64_t xs      : 2;
            // modify privilege
            uint64_t mprv    : 1;
            // permit supervisor user memory access
            uint64_t sum     : 1;
            // make executable readable
            uint64_t mxr     : 1;
            // trap virtual memory
            uint64_t tvm     : 1;
            // timeout wait (trap WFI)
            uint64_t tw      : 1;
            // trap SRET
            uint64_t tsr     : 1;
            uint64_t unused2 : 9;
            // U-mode XLEN
            uint64_t uxl     : 2;
            // S-mode XLEN
            uint64_t sxl     : 2;
            uint64_t unused3 : 27;
            // status dirty
            uint64_t sd      : 1;
        };

        uint64_t val;
    };

    mstatus_t(void) {
        val = 0;
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
            // Reserved Writes Preserve Values, Reads Ignore Values (WPRI)
            uint64_t wpri1  : 1;
            // interrupt enable
            uint64_t sie    : 1;
            uint64_t wpri12 : 3;
            // previous interrupt enable
            uint64_t spie   : 1;
            uint64_t ube    : 1;
            uint64_t wpri3  : 1;
            // previous mode (supervisor)
            uint64_t spp    : 1;
            uint64_t wpri4  : 4;
            // FPU status
            uint64_t fs     : 2;
            // extensions status
            uint64_t xs     : 2;
            uint64_t wpri5  : 1;
            // permit supervisor user memory access
            uint64_t sum    : 1;
            // make executable readable
            uint64_t mxr    : 1;
            uint64_t wpri6  : 12;
            // U-mode XLEN
            uint64_t uxl    : 2;
            uint64_t wpri7  : 29;
            // status dirty
            uint64_t sd     : 1;
        };

        uint64_t val;
    };

    sstatus_t(void) {
        val = 0;
        return;
    }

    sstatus_t(uint64_t _val) : val(_val) {
        return;
    }

    friend std::ostream&
    operator<<(std::ostream& _os, const sstatus_t& _sstatus) {
        printf("val: 0x%p, sie: %s, spie: %s, spp: %s", _sstatus.val,
               (_sstatus.sie == true ? "enable" : "disable"),
               (_sstatus.spie == true ? "enable" : "disable"),
               (_sstatus.spp == true ? "S mode" : "U mode"));
        return _os;
    }
};

/**
 * @brief 读取 sstatus 寄存器
 * @return uint64_t         读取到的值
 */
inline static sstatus_t READ_SSTATUS(void) {
    sstatus_t x;
    asm("csrr %0, sstatus" : "=r"(x));
    return x;
}

/**
 * @brief 写 sstatus 寄存器
 * @param  _x                要写的值
 */
inline static void WRITE_SSTATUS(sstatus_t _x) {
    asm("csrw sstatus, %0" : : "r"(_x));
}

/**
 * @brief 读 sip
 * @return uint64_t         读取到的值
 * @note Supervisor Interrupt Pending
 */
inline static uint64_t READ_SIP(void) {
    uint64_t x;
    asm("csrr %0, sip" : "=r"(x));
    return x;
}

/**
 * @brief 写 sip
 * @param  _x               要写的值
 */
inline static void WRITE_SIP(uint64_t _x) {
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
inline static uint64_t          READ_SIE(void) {
    uint64_t x;
    asm("csrr %0, sie" : "=r"(x));
    return x;
}

/**
 * @brief 写 sie
 * @param  _x                要写的值
 */
inline static void WRITE_SIE(uint64_t _x) {
    asm("csrw sie, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 sepc
 * @return uint64_t         读到的值
 * @note machine exception program counter, holds the instruction address to
 * which a return from exception will go.
 */
inline static uint64_t READ_SEPC(void) {
    uint64_t x;
    asm("csrr %0, sepc" : "=r"(x));
    return x;
}

/**
 * @brief 写 sepc
 * @param  _x               要写的值
 */
inline static void WRITE_SEPC(uint64_t _x) {
    asm("csrw sepc, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 stvec
 * @return uint64_t         读到的值
 * @note Supervisor Trap-Vector Base Address low two bits are mode.
 */
inline static uint64_t READ_STVEC(void) {
    uint64_t x;
    asm("csrr %0, stvec" : "=r"(x));
    return x;
}

/**
 * @brief 写 stvec
 * @param  _x               要写的值
 */
inline static void WRITE_STVEC(uint64_t _x) {
    asm("csrw stvec, %0" : : "r"(_x));
    return;
}

/// 中断模式 直接
static constexpr const uint64_t TVEC_DIRECT   = 0xFFFFFFFFFFFFFFFC;
/// 中断模式 向量
static constexpr const uint64_t TVEC_VECTORED = 0xFFFFFFFFFFFFFFFD;

/**
 * @brief 设置中断模式，直接
 */
inline static void              STVEC_DIRECT(void) {
    uint64_t stvec = READ_STVEC();
    stvec          = stvec & TVEC_DIRECT;
    WRITE_STVEC(stvec);
    return;
}

/**
 * @brief 设置中断模式，向量
 */
inline static void STVEC_VECTORED(void) {
    uint64_t stvec = READ_STVEC();
    stvec          = stvec & TVEC_VECTORED;
    WRITE_STVEC(stvec);
    return;
}

/**
 * @brief 设置页目录，仅更改 ppn
 * @param  _x               要设置的页目录
 * @note supervisor address translation and protection; holds the address of
 * the page table.
 * @todo 需要判断 _x 是否已经处理过
 */
inline static void SET_PGD(uintptr_t _x) {
    satp_t satp;
    // 读取现在的 pgd
    asm("csrr %0, satp" : "=r"(satp));
    // 更改 ppn
    // satp.ppn = _x & satp_t::PPN_MASK;
    satp.ppn = _x >> satp_t::PPN_OFFSET;
    // 写回
    asm("csrw satp, %0" : : "r"(satp));
    return;
}

/**
 * @brief 获取页目录，仅获取 ppn
 * @return uintptr_t        页目录
 */
inline static uintptr_t GET_PGD(void) {
    satp_t satp;
    asm("csrr %0, satp" : "=r"(satp));
    return satp.ppn << satp_t::PPN_OFFSET;
}

/**
 * @brief 开启分页
 * @return true             成功
 * @return false            失败
 */
inline static bool ENABLE_PG(void) {
    uintptr_t x = GET_PGD();
    satp_t    satp;
    satp.ppn  = x >> satp_t::PPN_OFFSET;
    satp.asid = 0;
    satp.mode = satp_t::SV39;
    asm("csrw satp, %0" : : "r"(satp));
    info("paging enabled.\n");
    return true;
}

/**
 * @brief 读 sscratch 寄存器
 * @param  _x                要写的值
 */
inline static uint64_t READ_SSCRATCH(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, sscratch" : "=r"(x));
    return x;
}

/**
 * @brief 写 sscratch 寄存器
 * @param  _x                要写的值
 */
inline static void WRITE_SSCRATCH(uint64_t _x) {
    asm("csrw sscratch, %0" : : "r"(_x));
    return;
}

/// [63]==1 interrupt, else exception
static constexpr const uint64_t CAUSE_INTR_MASK = 1ULL << 63;
/// low bits show code
static constexpr const uint64_t CAUSE_CODE_MASK = ~CAUSE_INTR_MASK;

/**
 * @brief 读 scause 寄存器 Supervisor Trap Cause
 * @return uint64_t         读到的值
 */
inline static uint64_t          READ_SCAUSE(void) {
    uint64_t x;
    asm("csrr %0, scause" : "=r"(x));
    return x;
}

/**
 * @brief 读 stval 寄存器 Supervisor Trap Value
 * @return uint64_t         读到的值
 */
inline static uint64_t READ_STVAL(void) {
    uint64_t x;
    asm("csrr %0, stval" : "=r"(x));
    return x;
}

/**
 * @brief 读 time 寄存器 supervisor-mode cycle counter
 * @return uint64_t         读到的值
 */
inline static uint64_t READ_TIME(void) {
    uint64_t x;
    // asm ("csrr %0, time" : "=r" (x) );
    // this instruction will trap in SBI
    asm("rdtime %0" : "=r"(x));
    return x;
}

/**
 * @brief 允许中断
 */
inline static void ENABLE_INTR(void) {
    WRITE_SSTATUS(READ_SSTATUS().val | SSTATUS_SIE);
    return;
}

/**
 * @brief 允许中断
 * @param  _sstatus         要设置的 sstatus
 */
inline static void ENABLE_INTR(sstatus_t& _sstatus) {
    _sstatus.sie = true;
    return;
}

/**
 * @brief 禁止中断
 */
inline static void DISABLE_INTR(void) {
    WRITE_SSTATUS(READ_SSTATUS().val & ~SSTATUS_SIE);
    return;
}

/**
 * @brief 禁止中断
 * @param  _sstatus         要设置的原 sstatus 值
 */
inline static void DISABLE_INTR(sstatus_t& _sstatus) {
    _sstatus.sie = false;
    return;
}

/**
 * @brief 读取中断状态
 * @return true             允许
 * @return false            禁止
 */
inline static bool STATUS_INTR(void) {
    sstatus_t x = READ_SSTATUS();
    return x.sie;
}

/**
 * @brief 读 sp 寄存器
 * @return uint64_t         读到的值
 */
inline static uint64_t READ_SP(void) {
    uint64_t x;
    asm("mv %0, sp" : "=r"(x));
    return x;
}

/**
 * @brief 读 tp 寄存器
 * @return uint64_t         读到的值
 */
inline static uint64_t READ_TP(void) {
    uint64_t x;
    asm("mv %0, tp" : "=r"(x));
    return x;
}

/**
 * @brief 写 tp 寄存器
 * @param  _x                要写的值
 */
inline static void WRITE_TP(uint64_t _x) {
    asm("mv tp, %0" : : "r"(_x));
    return;
}

/**
 * @brief 读 ra 寄存器
 * @return uint64_t         读到的值
 */
inline static uint64_t READ_RA(void) {
    uint64_t x;
    asm("mv %0, ra" : "=r"(x));
    return x;
}

/**
 * @brief 刷新 tlb
 */
inline static void VMM_FLUSH(uintptr_t) {
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

    friend std::ostream& operator<<(std::ostream& _os, const xregs_t& _xregs) {
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

    friend std::ostream& operator<<(std::ostream& _os, const fregs_t& _fregs) {
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
 * @brief 所有寄存器，在中断时使用，共 32+32+7=71 个
 */
struct all_regs_t {
    xregs_t   xregs;
    fregs_t   fregs;
    uintptr_t sepc;
    uintptr_t stval;
    uintptr_t scause;
    uintptr_t sie;
    sstatus_t sstatus;
    satp_t    satp;
    uintptr_t sscratch;

    friend std::ostream&
    operator<<(std::ostream& _os, const all_regs_t& _all_regs) {
        (void)_all_regs.fregs;
        _os << _all_regs.xregs << std::endl;
        printf("sepc: 0x%p, stval: 0x%p, scause: 0x%p, sie: 0x%p, sstatus: "
               "0x%p, satp: 0x%p, sscratch: 0x%p",
               _all_regs.sepc, _all_regs.stval, _all_regs.scause, _all_regs.sie,
               _all_regs.sstatus.val, _all_regs.satp.val, _all_regs.sscratch);
        return _os;
    }
};

};     // namespace CPU

#endif /* SIMPLEKERNEL_CPU_HPP */

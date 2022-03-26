
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

#include "stdint.h"
#include "stdbool.h"

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
    __asm__ volatile("csrr %0, satp" : "=r"(satp_old));
    // 如果开启了 sv39
    if (satp_old.mode == satp_t::SV39) {
        // 将新的页目录也设为开启
        satp_new.mode = satp_t::SV39;
    }
    __asm__ volatile("csrw satp, %0" : : "r"(satp_new));
    return;
}

/**
 * @brief 获取页目录
 * @return uintptr_t        页目录
 */
static inline uintptr_t GET_PGD(void) {
    satp_t satp;
    __asm__ volatile("csrr %0, satp" : "=r"(satp));
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
 * @brief 写 sscratch 寄存器
 * @param  _x                要写的值
 */
static inline void WRITE_SSCRATCH(uint64_t _x) {
    __asm__ volatile("csrw sscratch, %0" : : "r"(_x));
    return;
}

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
 * @brief 刷新 tlb
 */
static inline void VMM_FLUSH(uintptr_t) {
    // the zero, zero means flush all TLB entries.
    __asm__ volatile("sfence.vma zero, zero");
    return;
}

}; // namespace CPU

#endif /* _CPU_HPP_ */


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// cpu.hpp for Simple-XX/SimpleKernel.

#ifndef _CPU_HPP_
#define _CPU_HPP_

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

namespace CPU {
    // which hart (core) is this?
    static inline uint64_t READ_MHARTID(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mhartid" : "=r"(x));
        return x;
    }

    // Machine Status Register, mstatus
    // previous mode.
    static constexpr const uint64_t MSTATUS_MPP_MASK = 3 << 11;
    static constexpr const uint64_t MSTATUS_MPP_M    = 3 << 11;
    static constexpr const uint64_t MSTATUS_MPP_S    = 1 << 11;
    static constexpr const uint64_t MSTATUS_MPP_U    = 0 << 11;
    // machine-mode interrupt enable.
    static constexpr const uint64_t MSTATUS_MIE = 1 << 3;

    static inline uint64_t READ_MSTATUS(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mstatus" : "=r"(x));
        return x;
    }

    static inline void WRITE_MSTATUS(uint64_t x) {
        __asm__ volatile("csrw mstatus, %0" : : "r"(x));
        return;
    }

    // machine exception program counter, holds the
    // instruction address to which a return from
    // exception will go.
    static inline void WRITE_MEPC(uint64_t x) {
        __asm__ volatile("csrw mepc, %0" : : "r"(x));
        return;
    }

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

    static inline uint64_t READ_SSTATUS(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, sstatus" : "=r"(x));
        return x;
    }

    static inline void WRITE_SSTATUS(uint64_t x) {
        __asm__ volatile("csrw sstatus, %0" : : "r"(x));
    }

    // Supervisor Interrupt Pending
    static inline uint64_t READ_SIP(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, sip" : "=r"(x));
        return x;
    }

    static inline void WRITE_SIP(uint64_t x) {
        __asm__ volatile("csrw sip, %0" : : "r"(x));
        return;
    }

    // Supervisor Interrupt Enable
    // software
    static constexpr const uint64_t SIE_SSIE = 1 << 1;
    // timer
    static constexpr const uint64_t SIE_STIE = 1 << 5;
    // external
    static constexpr const uint64_t SIE_SEIE = 1 << 9;

    static inline uint64_t READ_SIE(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, sie" : "=r"(x));
        return x;
    }

    static inline void WRITE_SIE(uint64_t x) {
        __asm__ volatile("csrw sie, %0" : : "r"(x));
        return;
    }

    // Machine-mode Interrupt Enable
    // software
    static constexpr const uint64_t MIE_MSIE = 1 << 3;
    // timer
    static constexpr const uint64_t MIE_MTIE = 1 << 7;
    // external
    static constexpr const uint64_t MIE_MEIE = 1 << 11;

    static inline uint64_t READ_MIE(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mie" : "=r"(x));
        return x;
    }

    static inline void WRITE_MIE(uint64_t x) {
        __asm__ volatile("csrw mie, %0" : : "r"(x));
        return;
    }

    // machine exception program counter, holds the
    // instruction address to which a return from
    // exception will go.
    static inline uint64_t READ_SEPC(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, sepc" : "=r"(x));
        return x;
    }

    static inline void WRITE_SEPC(uint64_t x) {
        __asm__ volatile("csrw sepc, %0" : : "r"(x));
        return;
    }

    // Machine Exception Delegation
    static inline uint64_t READ_MEDELEG(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, medeleg" : "=r"(x));
        return x;
    }

    static inline void WRITE_MEDELEG(uint64_t x) {
        __asm__ volatile("csrw medeleg, %0" : : "r"(x));
        return;
    }

    // Machine Interrupt Delegation
    static inline uint64_t READ_MIDELEG(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mideleg" : "=r"(x));
        return x;
    }

    static inline void WRITE_MIDELEG(uint64_t x) {
        __asm__ volatile("csrw mideleg, %0" : : "r"(x));
        return;
    }

    // Supervisor Trap-Vector Base Address
    // low two bits are mode.
    static inline uint64_t READ_STVEC(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, stvec" : "=r"(x));
        return x;
    }

    static inline void WRITE_STVEC(uint64_t x) {
        __asm__ volatile("csrw stvec, %0" : : "r"(x));
        return;
    }

    // Machine-mode interrupt vector
    static inline uint64_t READ_MTVEC(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mtvec" : "=r"(x));
        return x;
    }

    static inline void WRITE_MTVEC(uint64_t x) {
        __asm__ volatile("csrw mtvec, %0" : : "r"(x));
        return;
    }

    static constexpr const uint64_t TVEC_DIRECT   = 0xFFFFFFFFFFFFFFFC;
    static constexpr const uint64_t TVEC_VECTORED = 0xFFFFFFFFFFFFFFFD;

    // direct mode
    static inline void MTVEC_DIRECT(void) {
        uint64_t mtvec = READ_MTVEC();
        mtvec          = mtvec & TVEC_DIRECT;
        WRITE_MTVEC(mtvec);
        return;
    }

    // Vectored mode
    static inline void MTVEC_VECTORED(void) {
        uint64_t mtvec = READ_MTVEC();
        mtvec          = mtvec & TVEC_VECTORED;
        WRITE_MTVEC(mtvec);
        return;
    }

    static inline void STVEC_DIRECT(void) {
        uint64_t stvec = READ_STVEC();
        stvec          = stvec & TVEC_DIRECT;
        WRITE_STVEC(stvec);
        return;
    }

    // Vectored mode
    static inline void STVEC_VECTORED(void) {
        uint64_t stvec = READ_STVEC();
        stvec          = stvec & TVEC_VECTORED;
        WRITE_STVEC(stvec);
        return;
    }

    // use riscv's sv39 page table scheme.
    static constexpr const uint64_t SATP_SV39 = (uint64_t)8 << 60;

    static constexpr void *SET_SV39(void *pgd) {
        return (void *)(SATP_SV39 | (((uint64_t)pgd) >> 12));
    }

    // supervisor address translation and protection;
    // holds the address of the page table.
    static inline void WRITE_SATP(void *x) {
        __asm__ volatile("csrw satp, %0" : : "r"(x));
        return;
    }

    static inline void *READ_SATP(void) {
        void *x;
        __asm__ volatile("csrr %0, satp" : "=r"(x));
        return x;
    }

    static inline bool ENABLE_PG(void) {
        void *x = READ_SATP();
        WRITE_SATP(SET_SV39(x));
        info("paging enabled.\n");
        return true;
    }

    // Supervisor Scratch register, for early trap handler in trampoline.S.
    static inline void WRITE_SSCRATCH(uint64_t x) {
        __asm__ volatile("csrw sscratch, %0" : : "r"(x));
        return;
    }

    static inline void WRITE_MSCRATCH(uint64_t x) {
        __asm__ volatile("csrw mscratch, %0" : : "r"(x));
        return;
    }

    // [31]=1 interrupt, else exception
    static constexpr const uint64_t CAUSE_INTR_MASK = 0x8000000000000000;
    // low bits show code
    static constexpr const uint64_t CAUSE_CODE_MASK = 0x7FFFFFFFFFFFFFFF;

    // Supervisor Machine Cause
    static inline uint64_t READ_MCAUSE(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mcause" : "=r"(x));
        return x;
    }

    // Supervisor Trap Cause
    static inline uint64_t READ_SCAUSE(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, scause" : "=r"(x));
        return x;
    }

    // Supervisor Trap Value
    static inline uint64_t READ_STVAL(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, stval" : "=r"(x));
        return x;
    }

    // Machine-mode Counter-Enable
    static inline uint64_t READ_MCOUNTEREN(void) {
        uint64_t x;
        __asm__ volatile("csrr %0, mcounteren" : "=r"(x));
        return x;
    }

    static inline void WRITE_MCOUNTEREN(uint64_t x) {
        __asm__ volatile("csrw mcounteren, %0" : : "r"(x));
        return;
    }

    // supervisor-mode cycle counter
    static inline uint64_t READ_TIME(void) {
        uint64_t x;
        // __asm__ volatile("csrr %0, time" : "=r" (x) );
        // this instruction will trap in SBI
        __asm__ volatile("rdtime %0" : "=r"(x));
        return x;
    }

    // enable device interrupts
    static inline void ENABLE_INTR(void) {
        WRITE_SSTATUS(READ_SSTATUS() | SSTATUS_SIE);
        return;
    }

    // disable device interrupts
    static inline void DISABLE_INTR(void) {
        WRITE_SSTATUS(READ_SSTATUS() & ~SSTATUS_SIE);
        return;
    }

    // are device interrupts enabled?
    static inline bool SSTATUS_INTR(void) {
        uint64_t x = READ_SSTATUS();
        return (x & SSTATUS_SIE) != 0;
    }

    static inline uint64_t READ_SP(void) {
        uint64_t x;
        __asm__ volatile("mv %0, sp" : "=r"(x));
        return x;
    }

    // read and write tp, the thread pointer, which holds
    // this core's hartid (core number), the index into cpus[].
    static inline uint64_t READ_TP(void) {
        uint64_t x;
        __asm__ volatile("mv %0, tp" : "=r"(x));
        return x;
    }

    static inline void WRITE_TP(uint64_t x) {
        __asm__ volatile("mv tp, %0" : : "r"(x));
        return;
    }

    static inline uint64_t READ_RA(void) {
        uint64_t x;
        __asm__ volatile("mv %0, ra" : "=r"(x));
        return x;
    }

    // flush the TLB.
    static inline void SFENCE_VMA(void) {
        // the zero, zero means flush all TLB entries.
        __asm__ volatile("sfence.vma zero, zero");
        return;
    }

};

#endif /* _CPU_HPP_ */

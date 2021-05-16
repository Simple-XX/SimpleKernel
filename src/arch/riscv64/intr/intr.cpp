
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"
#include "opensbi.h"

namespace INTR {
#ifdef __cplusplus
    extern "C" {
#endif

#ifdef __cplusplus
    }
#endif

    void enable_irq(uint32_t irq_no) {
        return;
    }

    void disable_irq(uint32_t irq_no) {
        return;
    }

// timer interrupt interval
#define INTERVAL (6000000)

    int32_t init(void) {
        // 设置 trap vector
        CPU::WRITE_STVEC((uint64_t)trap_entry);
        // 直接跳转到处理函数
        CPU::STVEC_DIRECT();
        // 开启时钟中断
        // 开启 STIE，允许时钟中断
        CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
        // 设置下一次时钟中断
        OPENSBI opensbi;
        opensbi.set_timer(CPU::READ_TIME() + INTERVAL);
        printf("intr init\n");
        return 0;
    }

    void trap_handler(void) {
        // CPU::DISABLE_INTR();
        // CPU::WRITE_SIE(CPU::READ_SIE() & ~CPU::SIE_STIE);
        OPENSBI opensbi;
        opensbi.set_timer(CPU::READ_TIME() + INTERVAL);
        CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_STIE);
        // CPU::WRITE_SSTATUS(CPU::READ_SSTATUS() | ~CPU::SSTATUS_SIE);
        // 中断原因
        uint64_t scause = CPU::READ_SCAUSE();
        // 异常返回值
        uint64_t sepc = CPU::READ_SEPC();
        // 中断具体信息
        uint64_t sstatus = CPU::READ_SSTATUS();
        printf("scause: %p\n", scause);
        printf("sepc: %p\n", sepc);
        printf("sstatus: %p\n", sstatus);
        if (scause & CPU::MCAUSE_INT_MASK) {
            // 中断
            // Index into 32-bit array containing addresses of functions
            printf("intr\n");
            // 跳转到对应的处理函数
            // async_handler[(mcause_value & CPU::MCAUSE_CODE_MASK)]();
        }
        else {
            // 异常
            // Branch to exception handler
            printf("excp\n");
            // 跳转到对应的处理函数
            // sync_handler[(mcause_value & CPU::MCAUSE_CODE_MASK)]();
        }
    }
};

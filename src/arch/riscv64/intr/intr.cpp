
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"

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

    int32_t init(void) {
        // 设置 trap vector
        CPU::WRITE_MTVEC((uint64_t)trap_entry);
        // 直接跳转到处理函数
        // CPU::MTVEC_DIRECT();
        printf("intr init\n");
        return 0;
    }

    void trap_handler(void) {
        uint64_t mcause = CPU::READ_MCAUSE();
        if (mcause & CPU::MCAUSE_INT_MASK) {
            // Branch to interrupt handler here
            // Index into 32-bit array containing addresses of functions
            // async_handler[(mcause_value & CPU::MCAUSE_CODE_MASK)]();
            printf("intr\n");
        }
        else {
            // Branch to exception handler
            // sync_handler[(mcause_value & CPU::MCAUSE_CODE_MASK)]();
            printf("excp\n");
        }
    }
};

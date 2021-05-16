
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
    // 最大中断数
    static constexpr const uint32_t INTERRUPT_MAX = 16;
    // 最大异常数
    static constexpr const uint32_t EXCP_MAX = 16;
    // 中断处理函数数组
    static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    // 异常处理函数数组
    static interrupt_handler_t excp_handlers[EXCP_MAX]
        __attribute__((aligned(4)));

    void enable_irq(uint32_t irq_no) {
        return;
    }

    void disable_irq(uint32_t irq_no) {
        return;
    }

    void register_interrupt_handler(uint8_t n, interrupt_handler_t h) {
        interrupt_handlers[n] = h;
        return;
    }

    void register_excp_handler(uint8_t n, interrupt_handler_t h) {
        excp_handlers[n] = h;
        return;
    }

    int32_t init(void) {
        // 设置 trap vector
        CPU::WRITE_STVEC((uint64_t)trap_entry);
        // 直接跳转到处理函数
        CPU::STVEC_DIRECT();
        printf("intr init\n");
        return 0;
    }

    void trap_handler(void) {
        // 中断原因
        uint64_t scause = CPU::READ_SCAUSE();
        // 异常返回值
        uint64_t sepc = CPU::READ_SEPC();
        // 中断具体信息
        uint64_t sstatus = CPU::READ_SSTATUS();
        // printf("scause: %p\n", scause);
        // printf("sepc: %p\n", sepc);
        // printf("sstatus: %p\n", sstatus);
        if (scause & CPU::CAUSE_INTR_MASK) {
            // 中断
            printf("intr: %s\n", intr_names[scause & CPU::CAUSE_CODE_MASK]);
            // 跳转到对应的处理函数
            interrupt_handlers[scause & CPU::CAUSE_CODE_MASK]();
        }
        else {
            // 异常
            // 跳转到对应的处理函数
            printf("excp: %s\n", excp_names[scause & CPU::CAUSE_CODE_MASK]);
        }
        return;
    }

};

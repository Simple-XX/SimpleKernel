
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"

namespace INTR {
    int32_t init(void) {
        // 内部中断初始化
        CLINT::init();
        // 外部中断初始化
        PLIC::init();
        printf("intr init\n");
        return 0;
    }

    void handler_default(void) {
        while (1) {
            ;
        }
        return;
    }

    void trap_handler(void) {
        // 中断原因
        uint64_t scause = CPU::READ_SCAUSE();
// #define DEBUG
#ifdef DEBUG
        // 异常返回值
        uint64_t sepc = CPU::READ_SEPC();
        // 中断具体信息
        uint64_t sstatus = CPU::READ_SSTATUS();
        printf("scause: 0x%X, sepc: 0x%X, sstatus: 0x%X\n", scause, sepc,
               sstatus);
#undef DEBUG
#endif
        if (scause & CPU::CAUSE_INTR_MASK) {
// 中断
#define DEBUG
#ifdef DEBUG
            printf("intr: %s\n",
                   CLINT::intr_names[scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
            // 跳转到对应的处理函数
            CLINT::do_interrupt(scause & CPU::CAUSE_CODE_MASK);
        }
        else {
// 异常
// 跳转到对应的处理函数
#define DEBUG
#ifdef DEBUG
            printf("excp: %s\n",
                   CLINT::excp_names[scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
            CLINT::do_excp(scause & CPU::CAUSE_CODE_MASK);
        }
        return;
    }
};

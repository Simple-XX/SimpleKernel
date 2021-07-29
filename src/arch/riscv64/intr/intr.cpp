
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"

// 栈，中断原因，中断返回值
extern "C" void trap_handler(uint64_t _scause, uint64_t _sepc,
                             uint64_t _stval) {

    // 消除 unused 警告
    (void)_sepc;
    (void)_stval;
#define DEBUG
#ifdef DEBUG
    printf("scause: 0x%p, sepc: 0x%p, stval: 0x%p.\n", _scause, _sepc, _stval);
#undef DEBUG
#endif
    if (_scause & CPU::CAUSE_INTR_MASK) {
// 中断
// #define DEBUG
#ifdef DEBUG
        printf("intr: %s.\n",
               CLINT::intr_names[_scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
        // 跳转到对应的处理函数
        CLINT::do_interrupt(_scause & CPU::CAUSE_CODE_MASK);
    }
    else {
// 异常
// 跳转到对应的处理函数
// #define DEBUG
#ifdef DEBUG
        printf("excp: %s.\n",
               CLINT::excp_names[_scause & CPU::CAUSE_CODE_MASK]);
#undef DEBUG
#endif
        CLINT::do_excp(_scause & CPU::CAUSE_CODE_MASK);
    }
    return;
}

// 默认使用的中断处理函数
void handler_default(void) {
    while (1) {
        ;
    }
    return;
}

INTR::INTR(void) {
    return;
}

INTR::~INTR(void) {
    return;
}

int32_t INTR::init(void) {
    // 内部中断初始化
    CLINT::init();
    // 外部中断初始化
    PLIC::init();
    printf("intr init.\n");
    return 0;
}

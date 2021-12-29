
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// timer.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "intr.h"

static uint64_t tick = 0;

static void timer_intr(INTR::intr_context_t *) {
    if (tick++ % 50 == 0) {
        printf("timer.\n");
    }
    return;
    return;
}

TIMER &TIMER::get_instance(void) {
    static TIMER timer;
    return timer;
}

void TIMER::init(void) {
    // 注册中断函数
    INTR::get_instance().register_interrupt_handler(INTR::IRQ0, timer_intr);
    // 开启时钟中断
    INTR::get_instance().enable_irq(INTR::IRQ0);
    info("timer init.\n");
}

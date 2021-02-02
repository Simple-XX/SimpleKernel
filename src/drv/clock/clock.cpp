
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// clock.cpp for Simple-XX/SimpleKernel.

#include "clock.h"
#include "io.h"

static void clock_handle(INTR::pt_regs_t *pt_regs __attribute__((unused))) {
    return;
}

CLOCK::CLOCK(void) {
    return;
}

CLOCK::~CLOCK(void) {
    return;
}

int32_t CLOCK::init(void) {
    uint32_t divisor = TIMER_FREQ / FREQUENCY;
    // 0x34
    io.outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    // 拆分低字节和高字节
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t hign = (uint8_t)((divisor >> 8) & 0xFF);
    // 分别写入低字节和高字节
    io.outb(IO_TIMER, low);
    io.outb(IO_TIMER, hign);
    set_clock_handle(&clock_handle);
    INTR::enable_irq(INTR::IRQ0);
    io.printf("clock init\n");
    return 0;
}

int32_t CLOCK::set_clock_handle(INTR::interrupt_handler_t h) {
    INTR::register_interrupt_handler(INTR::IRQ0, h);
    return 0;
}

CLOCK clock;

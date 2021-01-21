
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// clock.cpp for Simple-XX/SimpleKernel.

#include "clock.h"
#include "intr.h"
#include "io.h"

CLOCK::CLOCK(void) {
    uint32_t divisor = TIMER_FREQ / FREQUENCY;
    // 0x34
    iok.outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    // 拆分低字节和高字节
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t hign = (uint8_t)((divisor >> 8) & 0xFF);
    // 分别写入低字节和高字节
    iok.outb(IO_TIMER, low);
    iok.outb(IO_TIMER, hign);
    return;
}

CLOCK::~CLOCK(void) {
    return;
}

CLOCK clockk;

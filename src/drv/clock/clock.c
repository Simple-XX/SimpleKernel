
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// clock.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stddef.h"
#include "port.hpp"
#include "cpu.hpp"
#include "clock.h"

void clock_init(void) {
    uint32_t divisor = TIMER_FREQ / FREQUENCY;
    // 0x34
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    // 拆分低字节和高字节
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t hign = (uint8_t)((divisor >> 8) & 0xFF);
    // 分别写入低字节和高字节
    outb(IO_TIMER, low);
    outb(IO_TIMER, hign);

    printk_info("clock_init\n");
    return;
}

#ifdef __cplusplus
}
#endif

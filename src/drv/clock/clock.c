
// This file is a part of MRNIU/SimpleKernel
// (https://github.com/MRNIU/SimpleKernel).
//
// clock.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stddef.h"
#include "port.hpp"
#include "cpu.hpp"
#include "sync.hpp"
#include "sched.h"
#include "clock.h"

void clock_init(void) {
    bool intr_flag = false;
    local_intr_store(intr_flag);
    {
        uint32_t divisor = TIMER_FREQ / FREQUENCY;
        outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT); // 0x34
        // 拆分低字节和高字节
        uint8_t low  = (uint8_t)(divisor & 0xFF);
        uint8_t hign = (uint8_t)((divisor >> 8) & 0xFF);
        // 分别写入低字节和高字节
        outb(IO_TIMER, low);
        outb(IO_TIMER, hign);

        printk_info("clock_init\n");
    }
    local_intr_restore(intr_flag);
    return;
}

#ifdef __cplusplus
}
#endif

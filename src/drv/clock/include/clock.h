
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// clock.h for MRNIU/SimpleKernel.

#ifndef _CLOCK_H_
#define _CLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "port.hpp"
#include "debug.h"
#include "intr/include/intr.h"

#define FREQUENCY       100

// 定时中断由 8253/8254 芯片从 IRQ0 提供
#define IO_TIMER        0x40              // 8253 Timer #1

// 输入频率为 1193180，frequency 即每秒中断次数
#define TIMER_FREQ      1193180

// Intel 8253/8254 PIT芯片 I/O端口地址范围是40h~43h
#define TIMER_MODE      (IO_TIMER + 3)        // timer mode port

#define TIMER_SEL0      0x00                    // select counter 0
#define TIMER_RATEGEN   0x04                    // mode 2
#define TIMER_CLK       0x06                    // mode 3
#define TIMER_16BIT     0x30                   // r/w counter 16 bits, LSB first

void clock_init(void);
void clock_handler(pt_regs_t *);

#ifdef __cplusplus
}
#endif

#endif /* _CLOCK_H_ */

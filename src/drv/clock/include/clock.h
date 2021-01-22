
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// clock.h for Simple-XX/SimpleKernel.

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "stdint.h"

const uint32_t FREQUENCY = 100;
// 定时中断由 8253/8254 芯片从 IRQ0 提供
// 8253 Timer #1
const uint32_t IO_TIMER = 0x40;

// 输入频率为 1193180，frequency 即每秒中断次数
const uint32_t TIMER_FREQ = 1193180;

// Intel 8253/8254 PIT芯片 I/O端口地址范围是40h~43h
// timer mode port
const uint32_t TIMER_MODE = IO_TIMER + 3;

// select counter 0
const uint32_t TIMER_SEL0 = 0x00;
// mode 2
const uint32_t TIMER_RATEGEN = 0x04;
// mode 3
const uint32_t TIMER_CLK = 0x06;
// r/w counter 16 bits, LSB first
const uint32_t TIMER_16BIT = 0x30;

class CLOCK {
private:
protected:
public:
    CLOCK(void);
    ~CLOCK(void);
    int32_t init(void);
};

extern CLOCK clockk;

#endif /* _CLOCK_H_ */

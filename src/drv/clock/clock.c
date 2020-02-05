// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// clock.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "cpu.hpp"
#include "include/clock.h"
#include "sched/sched.h"


void clock_handler(pt_regs_t * regs UNUSED) {
	static uint32_t tick UNUSED = 0;
	// printk_color(light_green, "Tick: %d\n", tick++);
	schedule();
}

void clock_init(void) {
	cpu_cli();
	uint32_t divisor = TIMER_FREQ / FREQUENCY;
	outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT); // 0x34
	// 拆分低字节和高字节
	uint8_t low = (uint8_t)(divisor & 0xFF);
	uint8_t hign = (uint8_t)( (divisor >> 8) & 0xFF);
	// 分别写入低字节和高字节
	outb(IO_TIMER, low);
	outb(IO_TIMER, hign);
	// 注册时间相关的处理函数
	register_interrupt_handler(IRQ0, &clock_handler);
	enable_irq(IRQ0);

	printk_info("clock_init\n");
	cpu_sti();
	return;
}

#ifdef __cplusplus
}
#endif

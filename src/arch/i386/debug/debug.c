
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"
#include "cpu.hpp"
#include "multiboot2.h"



void debug_init(uint32_t magic, uint32_t addr) {
		printk_color(COL_DEBUG, "[DEBUG]");
		printk("debug_init\n");
		multiboot2_init(magic, addr);
		return;
}

void print_cur_status(){
		static int round = 0;
		uint16_t reg1, reg2, reg3, reg4;
		asm volatile (  "mov %%cs, %0;"
		                "mov %%ds, %1;"
		                "mov %%es, %2;"
		                "mov %%ss, %3;"
		                : "=m" (reg1), "=m" (reg2), "=m" (reg3), "=m" (reg4));

		// 打印当前的运行级别
		printk("%d:  @ring %d\n", round, reg1 & 0x3);
		printk("%d:  cs = %X\n", round, reg1);
		printk("%d:  ds = %X\n", round, reg2);
		printk("%d:  es = %X\n", round, reg3);
		printk("%d:  ss = %X\n", round, reg4);
		++round;
}


// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"
#include "cpu.hpp"


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
		printk("%d:  cs = %x\n", round, reg1);
		printk("%d:  ds = %x\n", round, reg2);
		printk("%d:  es = %x\n", round, reg3);
		printk("%d:  ss = %x\n", round, reg4);
		++round;
}


// void debug_init(uint64_t magic, multiboot_info_t * mb) {
//
//
//   // 从 GRUB 提供的信息中获取到内核符号表和代码地址信息
//
//   return;
// }

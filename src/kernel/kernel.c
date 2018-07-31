
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"

// 内核入口
void kernel_main()
{
	// 系统初始化
	debug_init();
	terminal_init();	// 控制台初始化
	gdt_init();	// GDT 初始化
	idt_init();	// IDT 初始化



	// 输出一些基本信息
	printk("Welcome to my kernel.\n");
	printk("terminal_color: %d\n", terminal_color);
	printk("current cursor pos: %d\n", terminal_getcursorpos());
	show_memory_map();


	printk("kernel in memory start: 0x%08X\n", kern_start);
	printk("kernel in memory end:   0x%08X\n", kern_end);
	printk("kernel in memory size:   %d KB\n\n", (kern_end - kern_start + 1023) / 1024);


	asm volatile("sti");
	clock_init();


	printk("End.\n");

	return;
}

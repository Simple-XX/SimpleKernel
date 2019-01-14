
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"

// 内核入口
void kernel_main()
{
		// 系统初始化
		debug_init();
		console_init(); // 控制台初始化
		gdt_init(); // GDT 初始化
		idt_init(); // IDT 初始化
		clock_init(); // 时钟初始化



		// 输出一些基本信息
		printk_color(magenta,"Welcome to my kernel.\n");
		printk_color(light_red,"kernel in memory start: 0x%08X\n", kernel_start);
		printk_color(light_red,"kernel in memory end: 0x%08X\n", kernel_end);
		printk_color(light_red,"kernel in memory size: %d KB, %d pages\n",
		             (kernel_end - kernel_start) / 1024, (kernel_end - kernel_start) / 1024 / 4);



		cpu_sti();
		printk_color(white, "sti\n");
		if(canint())
				printk_color(white, "interrupt accept!\n");
		else
				printk_color(light_red, "interrupt not accept!\n");

		//cpu_cli();// close intr
		for(int i=0; i<5; i++) {
				if(canint())
						printk_color(white, "before hlt, interrupt accept!\n");
				else
						printk_color(light_red, "before hlt, interrupt not accept!\n");

				cpu_hlt();

				if(canint())
						printk_color(white, "after hlt, interrupt accept!\n\n");
				else
						printk_color(light_red, "after hlt, interrupt not accept!\n\n");
		}
		//cpu_hlt();
		//cpu_cli();

		if(canint())
				printk_color(white, "interrupt accept!\n");
		else
				printk_color(light_red, "interrupt not accept!\n");


		printk_color(white, "\nEnd.\n");
		return;
}

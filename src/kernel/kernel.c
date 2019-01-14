
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"
#include "assert.h"

extern void tmp();

// 内核入口
void kernel_main(void);
void kernel_main(){
		// 系统初始化
		debug_init();
		console_init();       // 控制台初始化
		gdt_init();       // GDT 初始化
		idt_init();       // IDT 初始化

		clock_init();       // 时钟初始化
		keyboard_init();       // 键盘初始化
		showinfo();


		while(1);

		// printk_color(white, "\nEnd.\n");
		return;
}

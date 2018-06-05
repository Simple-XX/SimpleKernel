
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.


#include "../include/kernel.h"
#include "../include/vga.h"
#include "../include/tty.h"
#include "../include/port.h"


// 内核入口
void kernel_main(void)
{
	// 系统初始化
	terminal_init();	// 控制台初始化
	gdt_init();	// GDT 初始化


	// 输出一些基本信息
	printk("Welcome to my kernel.\n");
	printk("terminal_color: %d\n", terminal_color);
	printk("current cursor pos: %d\n", terminal_getcursorpos());


}

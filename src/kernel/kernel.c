
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"

// 内核入口
void kernel_main(uint32_t magic, uint32_t addr) {
		console_init();       // 控制台初始化
		debug_init(magic, addr);
		gdt_init();        // GDT 初始化
		idt_init();       // IDT 初始化
		clock_init();       // 时钟初始化
		keyboard_init();       // 键盘初始化
		pmm_init(addr);
		showinfo();

		uint32_t allc_addr = NULL;
		printk_color(red, "Test Physical Memory Alloc :\n");
		allc_addr = pmm_alloc_page();
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		allc_addr = pmm_alloc_page();
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		allc_addr = pmm_alloc_page();
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		allc_addr = pmm_alloc_page();
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);


		while(1);

		return;
}

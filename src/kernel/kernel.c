
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"
#include "assert.h"


// 内核入口
// magic 魔数 addr multiboot 结构体的地址
void kernel_main(uint32_t magic, uint32_t addr);
void kernel_main(uint32_t magic, uint32_t addr) {
		multiboot_info_t * mboot_ptr = (multiboot_info_t *)addr;
		// 系统初始化

		terminal_init();       // 控制台初始化
		gdt_init();        // GDT 初始化
		idt_init();       // IDT 初始化

		clock_init();       // 时钟初始化
		keyboard_init();       // 键盘初始化
		showinfo();
		debug_init(magic, mboot_ptr);
		// print_cur_status();
		// panic("test");


		while(1);

		printk_color(white, "\nEnd.\n");
		return;


}


// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/multiboot2.h"
#include "mm/include/vmm.h"

__attribute__( ( section(".init.text") ) ) void kernel_entry(uint32_t magic, uint32_t addr);
extern void kernel_main(uint32_t magic, uint32_t addr);
void showinfo(void);

// 内核入口函数
void kernel_entry(uint32_t magic, uint32_t addr) {

	console_init(); // 控制台初始化
	multiboot2_init(magic, addr); // 从 multiboot 获得系统初始信息
	gdt_init(); // GDT 初始化
	idt_init(); // IDT 初始化
	clock_init(); // 时钟初始化
	keyboard_init(); // 键盘初始化
	pmm_init();
	vmm_init();
	debug_init(magic, addr);
	showinfo();

	// test();

	kernel_main(magic, addr);
	return;
}

void showinfo(void) {
	// 输出一些基本信息
	printk_color(magenta,"SimpleKernel\n");
	printk_color(light_red,"kernel in memory start: 0x%08X\n", kernel_start);
	printk_color(light_red,"kernel in memory end: 0x%08X\n", kernel_end);
	printk_color(light_red,"kernel in memory size: %d KB, %d pages\n",
	             ( kernel_end - kernel_start ) / 1024, ( kernel_end - kernel_start ) / 1024 / 4);
}

#ifdef __cplusplus
}
#endif

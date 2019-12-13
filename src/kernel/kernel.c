
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/kernel.h"
#include "../test/include/test.h"

// 内核入口
// 指针是 32 位的
void kernel_main(ptr_t magic, ptr_t addr) {
	console_init(); // 控制台初始化
	multiboot2_init(magic, addr); // 从 multiboot 获得系统初始信息
	gdt_init(); // GDT 初始化
	idt_init(); // IDT 初始化
	clock_init(); // 时钟初始化
	keyboard_init(); // 键盘初始化
	debug_init(magic, addr);
	pmm_init();
	vmm_init();

	showinfo();



	while (1);

	return;
}

#ifdef __cplusplus
}
#endif


// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/kernel.h"
#include "../test/include/test.h"
#include "debug.h"

// 内核入口
void kernel_main(ptr_t magic, ptr_t addr) {
	// 控制台初始化
	console_init();
	// 从 multiboot 获得系统初始信息
	multiboot2_init(magic, addr);
	// GDT、IDT 初始化
	arch_init();
	// 时钟初始化
	clock_init();
	// 键盘初始化
	keyboard_init();
	debug_init(magic, addr);
	// 物理内存初始化
	pmm_init();
	// 虚拟内存初始化
	vmm_init();
	// 堆初始化
	// heap_init();

	// showinfo();
	test();

	while(1);

	return;
}

#ifdef __cplusplus
}
#endif

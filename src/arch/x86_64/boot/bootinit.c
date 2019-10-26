
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/multiboot2.h"

__attribute__( ( section(".init.text") ) ) void kernel_entry(uint32_t magic, uint32_t addr);
extern void kernel_main(uint32_t magic, uint32_t addr);

// 内核入口函数
void kernel_entry(uint32_t magic, uint32_t addr) {

	console_init(); // 控制台初始化
	gdt_init(); // GDT 初始化
	idt_init(); // IDT 初始化

	kernel_main(magic, addr);
	return;
}


#ifdef __cplusplus
}
#endif

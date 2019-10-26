
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// init.c for MRNIU/SimpleKernel.


// #ifdef __cplusplus
// extern "C" {
// #endif
//
// #include "include/init.h"
// #include "console/include/console.h"
// // 内核入口函数
//
// void kernel_entry(uint32_t magic, uint32_t addr) {
// 	uint64_t cr4;
// #ifdef __x86_64__
// 	__asm__ volatile ( "movq %%cr3, %0" : "=r" ( cr4 ) );
// #else
// 	__asm__ volatile ( "mov %%cr3, %0" : "=r" ( cr4 ) );
// #endif
// 	cr4 |= ( 1 << 5 );
// #ifdef __x86_64__
// 	__asm__ volatile ( "movq %0, %%cr4" : : "r" ( cr4 ) );
// #else
// 	__asm__ volatile ( "mov %0, %%cr4" : : "r" ( cr4 ) );
// #endif
//
// 	__asm__ volatile ( "mov %0, %%cr3" : : "r" ( pgd_tmp ) );
// 	__asm__ volatile ( "mov $0xC0000080, %%ecx" : : );
// 	__asm__ volatile ( "rdmsr" : : );
// 	__asm__ volatile ( "or $(1<<8), %%eax" : : );
// 	__asm__ volatile ( "wrmsr" : : );
//
// 	// __asm__ volatile ( "mov %%cr0, %%eax" : : );
// 	// __asm__ volatile ( "or $(1 << 31), %%eax" : : );
// 	// __asm__ volatile ( "mov %%eax, %%cr0" : : );
//
// 	console_init(); // 控制台初始化
// 	gdt_init(); // GDT 初始化
// 	idt_init(); // IDT 初始化
//
// 	kernel_main(magic, addr);
// 	return;
// }
//
//
// #ifdef __cplusplus
// }
// #endif

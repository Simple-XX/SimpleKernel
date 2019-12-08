
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.h for MRNIU/SimpleKernel.


#ifndef _KERNEL_H_
#define _KERNEL_H_

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined( __linux__ )
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
// #if !defined( __i386__ ) | !defined(__x86_64__)
// #error "Please compile that with a ix86-elf compiler"
// #endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "multiboot2.h"
#include "mm/include/pmm.h"


void kernel_main(uint32_t magic, uint32_t addr);
void console_init(void);
void gdt_init(void);
void idt_init(void);
void clock_init(void);
void keyboard_init(void);
void mouse_init(void);
void debug_init(uint32_t magic, uint32_t addr);
void pmm_init(void);
void vmm_init(void);
void showinfo(void);

void showinfo(void) {
	// 输出一些基本信息
	printk_color(magenta,"SimpleKernel\n");
	printk_color(light_red,"kernel in memory start: 0x%08X\n", kernel_start);
	printk_color(light_red,"kernel in memory end: 0x%08X\n", kernel_end);
	printk_color(light_red,"kernel in memory size: %d KB, %d pages\n",
	             ( kernel_end - kernel_start ) / 1024, ( kernel_end - kernel_start ) / 1024 / 4);
}

#endif /* _KERNEL_H_ */

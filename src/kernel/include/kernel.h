
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
#include "mem/pmm.h"
#include "mem/vmm.h"

void kernel_main(void);
void console_init(void);
void gdt_init(void);
void idt_init(void);
void clock_init(void);
void keyboard_init(void);
void mouse_init(void);
void debug_init(void);
void pmm_init(void);
void vmm_init(void);
void showinfo(void);

void showinfo(void) {
	// 输出一些基本信息
	printk_color(magenta,"SimpleKernel\n");

	printk_info("kernel in memory(VMA=LMA-0xC0000000) start: 0x%08X, end 0x%08X\n", kernel_start, kernel_end);
	printk_info(".text in memory(VMA=LMA-0xC0000000) start: 0x%08X, end 0x%08X\n", kernel_text_start, kernel_text_end);
	printk_info(".data in memory(VMA=LMA-0xC0000000) start: 0x%08X, end 0x%08X\n", kernel_data_start, kernel_data_end);
	printk_info("kernel in memory size: %d KB, %d pages\n",
	            ( kernel_end - kernel_start ) / 1024, ( kernel_end - kernel_start ) / 1024 / 4);
}

#endif /* _KERNEL_H_ */

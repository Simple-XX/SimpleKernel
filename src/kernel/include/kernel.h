
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.h for MRNIU/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined (__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
// #if !defined( __i386__ ) | !defined(__x86_64__)
// #error "Please compile that with a ix86-elf compiler"
// #endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "multiboot2.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "heap/heap.h"

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
void heap_init(void);
void showinfo(void);

void showinfo(void) {
	// 输出一些基本信息
	printk_color(magenta, "SimpleKernel\n");
	printk_info("kernel init in memory(VMA==LMA) start: 0x%08X, end 0x%08X\n", &kernel_init_start, &kernel_init_end);
	printk_info(".init.text in memory(VMA==LMA) start: 0x%08X, end 0x%08X\n", &kernel_init_text_start, &kernel_init_text_end);
	printk_info(".init.data in memory(VMA==LMA) start: 0x%08X, end 0x%08X\n", &kernel_init_data_start, &kernel_init_data_end);
	printk_info("kernel init in memory size: %d KB, %d pages\n",
		(&kernel_init_end - &kernel_init_start) / 1024, (&kernel_init_end - &kernel_init_start) / 1024 / 4);

	printk_info("kernel in memory(VMA=LMA+0xC0000000) start: 0x%08X, end 0x%08X\n", &kernel_start, &kernel_end);
	printk_info(".text in memory(VMA=LMA+0xC0000000) start: 0x%08X, end 0x%08X\n", &kernel_text_start, &kernel_text_end);
	printk_info(".data in memory(VMA=LMA+0xC0000000) start: 0x%08X, end 0x%08X\n", &kernel_data_start, &kernel_data_end);
	printk_info("kernel in memory size: %d KB, %d pages\n",
		(&kernel_end - &kernel_start) / 1024, (&kernel_end - &kernel_start) / 1024 / 4);
}

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_H_ */


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

void kernel_main(void);
void console_init(void);
void gdt_init(void);
void intr_init(void);
void clock_init(void);
void keyboard_init(void);
void debug_init(void);
void showinfo(void);

extern ptr_t kernel_start[];
extern ptr_t kernel_text_start[];
extern ptr_t kernel_text_end[];
extern ptr_t kernel_data_start[];
extern ptr_t kernel_data_end[];
extern ptr_t kernel_end[];

void showinfo(void) {
    // 输出一些基本信息
    printk_color(magenta, "SimpleKernel\n");

    printk_info("kernel in memory(VMA==LMA) start: 0x%08X, end 0x%08X\n",
                kernel_start, kernel_end);
    printk_info(".text in memory(VMA==LMA) start: 0x%08X, end 0x%08X\n",
                kernel_text_start, kernel_text_end);
    printk_info(".data in memory(VMA==LMA) start: 0x%08X, end 0x%08X\n",
                kernel_data_start, kernel_data_end);
    printk_info("kernel in memory size: %d KB, %d pages\n",
                (kernel_end - kernel_start) / 1024,
                (kernel_end - kernel_start) / 1024 / 4);
}

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_H_ */

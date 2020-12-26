
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
#include "stdlib.h"
#include "console.h"
#include "multiboot2.h"
#include "arch_init.h"
#include "pmm.h"

void kernel_main(uint32_t magic, uint32_t addr);

void showinfo(void);
void showinfo(void) {
    // 输出一些基本信息
    printk_color(magenta, "SimpleKernel\n");
    printk_info("kernel in memory(VMA==LMA) start: 0x%X, end 0x%X\n",
                KERNEL_START_ADDR, KERNEL_END_ADDR);
    printk_info("kernel in memory(VMA==LMA) align 4k start: 0x%X, end "
                "0x%X\n",
                kernel_start_align4k, kernel_end_align4k);
    printk_info("kernel in memory size: 0x%X KB, 0x%X pages\n",
                (KERNEL_END_ADDR - KERNEL_START_ADDR) / 1024,
                ALIGN4K(KERNEL_END_ADDR - KERNEL_START_ADDR) / PMM_PAGE_SIZE);
}

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_H_ */


// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.h for MRNIU/SimpleKernel.


#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stdio.h"
#include "multiboot2.h"
#include "stddef.h"


#define STACK_SIZE    8192
#define PMM_MAX_SIZE  0x40000000
#define PMM_PAGE_SIZE 0x1000      // 页面大小 4KB
#define PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)

extern uint32_t kernel_start[];
extern uint32_t kernel_end[];


// 初始化内存管理
void pmm_init(struct multiboot_tag *tag);

#endif

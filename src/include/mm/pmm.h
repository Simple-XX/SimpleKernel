
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.h for MRNIU/SimpleKernel.


#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stdio.h"



#define STACK_SIZE    8192
#define KERN_STACK_SIZE 2048      // 内核栈大小
#define PMM_MAX_SIZE  0x20000000  // 可用内存 512MB
#define PMM_PAGE_SIZE 0x1000      // 页面大小 4KB
#define PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)

static uint64_t pmm_stack[PAGE_MAX_SIZE+1]; // 物理内存页面管理的栈
static uint64_t pmm_stack_top;  // 物理内存管理的栈指针
uint64_t phy_page_count; // 物理内存页的数量

extern uint8_t kernel_start[];
extern uint8_t kernel_end[];


// 初始化内存管理
void pmm_init(void);

#endif

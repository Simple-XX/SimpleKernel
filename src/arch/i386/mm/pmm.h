
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.h for MRNIU/SimpleKernel.


#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stdio.h"
#include "multiboot2.h"
#include "stddef.h"


#define STACK_SIZE    0x1000  // 4096
#define PMM_MAX_SIZE  0x40000000 // 4 gib
#define PMM_PAGE_SIZE 0x1000      // 页面大小 4KB
#define PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)

extern uint32_t kernel_start[];
extern uint32_t kernel_end[];

// 初始化内存管理
void pmm_init(multiboot_tag_t * tag);

uint32_t pmm_alloc_page(void);

uint32_t pmm_alloc_pages(uint32_t size);

void pmm_free_page(uint32_t page);
#endif

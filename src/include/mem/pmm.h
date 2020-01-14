
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// pmm.h for MRNIU/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stddef.h"
#include "debug.h"
#include "multiboot2.h"

#define STACK_SIZE    (0x1000U)  // 4096
#define PMM_MAX_SIZE  (0x20000000U)  // 512 MB

// 内核的偏移地址
#define KERNEL_BASE    (0xC0000000U)
// 内核占用大小 4MB
#define KERNEL_SIZE    (0x400000U)
// 映射内核需要的页数
#define PMM_PAGES_KERNEL    (KERNEL_SIZE / PMM_PAGE_SIZE)

// 页掩码，用于 4KB 对齐
#define PMM_PAGE_MASK    (0xFFFFF000U)

// PAE 标志的处理
#ifdef CPU_PAE
#else
#endif

// PSE 标志的处理
#ifdef CPU_PSE
// 页大小 4MB
#define PMM_PAGE_SIZE    (0x400000U)
#else
// 页大小 4KB
#define PMM_PAGE_SIZE    (0x1000U)
#endif

#define PMM_PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)// 物理页数量 131072, 0x20000

extern ptr_t kernel_init_start[];
extern ptr_t kernel_init_text_start[];
extern ptr_t kernel_init_text_end[];
extern ptr_t kernel_init_data_start[];
extern ptr_t kernel_init_data_end[];
extern ptr_t kernel_init_end[];

extern ptr_t kernel_start[];
extern ptr_t kernel_text_start[];
extern ptr_t kernel_text_end[];
extern ptr_t kernel_data_start[];
extern ptr_t kernel_data_end[];
extern ptr_t kernel_end[];

// 开启分页机制之后的内核栈
extern uint8_t kernel_stack[STACK_SIZE];
// 内核栈的栈顶
extern ptr_t kernel_stack_top;

extern multiboot_memory_map_entry_t * mmap_entries;
extern multiboot_mmap_tag_t * mmap_tag;

// 初始化内存管理
void pmm_init(void);

ptr_t pmm_alloc(uint32_t byte);

void pmm_free_page(ptr_t page);

#ifdef __cplusplus
}
#endif

#endif /* _PMM_H_ */

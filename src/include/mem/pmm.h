
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

#define STACK_SIZE    (0x1000UL)  // 4096
#define PMM_MAX_SIZE  (0x20000000UL)  // 512 MB

// 内核的偏移地址
#define KERNEL_BASE    (0xC0000000UL)
// 内核占用大小 4MB
#define KERNEL_SIZE    (0x400000UL)
// 映射内核需要的页数
#define PMM_PAGES_KERNEL    (KERNEL_SIZE / PMM_PAGE_SIZE)

// 页掩码，用于 4KB 对齐
#define PMM_PAGE_MASK    (0xFFFFF000UL)

// PAE 标志的处理
#ifdef CPU_PAE
#else
#endif

// PSE 标志的处理
#ifdef CPU_PSE
// 页大小 4MB
#define PMM_PAGE_SIZE    (0x400000UL)
#else
// 页大小 4KB
#define PMM_PAGE_SIZE    (0x1000UL)
#endif

#define PMM_PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)// 物理页数量 131072, 0x20000

// A common problem is getting garbage data when trying to use a value defined in a linker script.
// This is usually because they're dereferencing the symbol. A symbol defined in a linker script (e.g. _ebss = .;)
// is only a symbol, not a variable. If you access the symbol using extern uint32_t _ebss;
// and then try to use _ebss the code will try to read a 32-bit integer from the address indicated by _ebss.
// The solution to this is to take the address of _ebss either by using it as & _ebss or by defining it as
// an unsized array(extern char _ebss[]; ) and casting to an integer.(The array notation prevents accidental
// reads from _ebss as arrays must be explicitly dereferenced)
// ref: http://wiki.osdev.org/Using_Linker_Script_Values
extern ptr_t * kernel_init_start;
extern ptr_t * kernel_init_text_start;
extern ptr_t * kernel_init_text_end;
extern ptr_t * kernel_init_data_start;
extern ptr_t * kernel_init_data_end;
extern ptr_t * kernel_init_end;

extern ptr_t * kernel_start;
extern ptr_t * kernel_text_start;
extern ptr_t * kernel_text_end;
extern ptr_t * kernel_data_start;
extern ptr_t * kernel_data_end;
extern ptr_t * kernel_end;

// 开启分页机制之后的内核栈
extern uint8_t kernel_stack[STACK_SIZE];
// 内核栈的栈顶
extern ptr_t kernel_stack_top;

extern multiboot_memory_map_entry_t * mmap_entries;
extern multiboot_mmap_tag_t * mmap_tag;

// 初始化内存管理
void pmm_init(void);

ptr_t pmm_alloc(size_t byte);

void pmm_free_page(ptr_t page);

#ifdef __cplusplus
}
#endif

#endif /* _PMM_H_ */

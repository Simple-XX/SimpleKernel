
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// pmm.h for MRNIU/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"
#include "e820.h"
#include "multiboot2.h"

// 32KB
#define STACK_SIZE    (0x8000UL)
#define STACK_PAGES   (STACK_SIZE / PMM_PAGE_SIZE)
// 512 MB
#define PMM_MAX_SIZE  (0x20000000UL)

// 内核的偏移地址
#define KERNEL_BASE    (0xC0000000UL)
// 内核占用大小 8MB
#define KERNEL_SIZE    (0x800000UL)
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

// 开启分页机制之后的内核栈顶(低地址)
extern ptr_t kernel_stack_top[STACK_SIZE];
// 内核栈的栈底(高地址)
extern ptr_t kernel_stack_bottom;

extern multiboot_memory_map_entry_t * mmap_entries;
extern multiboot_mmap_tag_t * mmap_tag;

// 内存管理结构体
typedef
    struct pmm_manage {
	// 管理算法的名称
	const char *      name;
	// 初始化
	void (* pmm_manage_init)(ptr_t page_start, uint32_t page_count);
	// 申请物理内存，单位为 Byte
	ptr_t (* pmm_manage_alloc)(uint32_t bytes);
	// 释放内存页
	void (* pmm_manage_free)(ptr_t addr_start, uint32_t bytes);
	// 返回当前可用内存页数量
	uint32_t (* pmm_manage_free_pages_count)(void);
} pmm_manage_t;

// 物理内存初始化
void pmm_phy_init(e820map_t * e820map);
// 物理内存管理初始化
void pmm_mamage_init(e820map_t * e820map);
// 初始化内存管理
void pmm_init(void);
// 分配内存，单位为 byte，返回的是物理地址
ptr_t pmm_alloc(size_t byte);
// 回收内存，单位为 byte
void pmm_free(ptr_t addr, uint32_t byte);
// 返回可用物理内存页数
uint32_t pmm_free_pages_count(void);

#ifdef __cplusplus
}
#endif

#endif /* _PMM_H_ */


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.h for Simple-XX/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"
#include "multiboot2.h"

// A common problem is getting garbage data when trying to use a value
// defined in a linker script. This is usually because they're dereferencing
// the symbol. A symbol defined in a linker script (e.g. _ebss = .;) is only
// a symbol, not a variable. If you access the symbol using extern uint32_t
// _ebss; and then try to use _ebss the code will try to read a 32-bit
// integer from the address indicated by _ebss. The solution to this is to
// take the address of _ebss either by using it as & _ebss or by defining it
// as an unsized array(extern char _ebss[]; ) and casting to an integer.(The
// array notation prevents accidental reads from _ebss as arrays must be
// explicitly dereferenced) ref:
// http://wiki.osdev.org/Using_Linker_Script_Values
extern addr_t *kernel_start;
extern addr_t *kernel_text_start;
extern addr_t *kernel_text_end;
extern addr_t *kernel_data_start;
extern addr_t *kernel_data_end;
extern addr_t *kernel_end;

#define KERNEL_START_ADDR (&kernel_start)
#define KERNEL_TEXT_START_ADDR (&kernel_text_start)
#define KERNEL_TEXT_END_ADDR (&kernel_text_end)
#define KERNEL_DATA_START_ADDR (&kernel_data_start)
#define KERNEL_DATA_END_ADDR (&kernel_date_end)
#define KERNEL_END_ADDR (&kernel_end)

// 内核栈大小 8KB
#define KERNEL_STACK_SIZE (0x2000UL)
// 内核栈需要的内存页数
#define KERNEL_STACK_PAGES (KERNEL_STACK_SIZE / PMM_PAGE_SIZE)
// 内核栈开始地址，内核结束后
#define KERNEL_STACK_START (((addr_t)(KERNEL_END_ADDR)) & PMM_PAGE_MASK)
// 内核栈结束地址
#define KERNEL_STACK_END (KERNEL_STACK_START + KERNEL_STACK_SIZE)
// 物理内存大小 2GB
#define PMM_MAX_SIZE (0x80000000UL)

// 内核的偏移地址
#define KERNEL_BASE (0x0UL)
// 内核占用大小 8MB
#define KERNEL_SIZE (0x800000UL)
// 映射内核需要的页数
#define PMM_PAGES_KERNEL (KERNEL_SIZE / PMM_PAGE_SIZE)

// 页掩码，用于 4KB 对齐
#define PMM_PAGE_MASK (0xFFFFF000UL)

// 对齐
#define ALIGN4K(x)                                                             \
    (((addr_t)(x) % PMM_PAGE_SIZE == 0)                                        \
         ? (addr_t)(x)                                                         \
         : (((addr_t)(x) + PMM_PAGE_SIZE - 1) & PMM_PAGE_MASK))

// PAE 标志的处理
#ifdef CPU_PAE
#else
#endif

// PSE 标志的处理
#ifdef CPU_PSE
// 页大小 4MB
#define PMM_PAGE_SIZE (0x400000UL)
#else
// 页大小 4KB
#define PMM_PAGE_SIZE (0x1000UL)
#endif

// 物理页数量 131072,
// 0x20000，除去外设映射，实际可用物理页数量为 159 + 130800 =
// 130959 (这是物理内存为 512MB 的情况)
#define PMM_PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)

// 4k 对齐的内核开始与结束地址
extern addr_t kernel_start_align4k;
extern addr_t kernel_end_align4k;

extern multiboot_memory_map_entry_t *mmap_entries;
extern multiboot_mmap_tag_t *        mmap_tag;

// 物理页结构体
typedef struct physical_page {
    // 起始地址
    addr_t addr;
    // 该页被引用次数，-1 表示此页内存被保留，禁止使用
    int32_t ref;
} physical_page_t;

// 可用内存的物理页数组
extern physical_page_t mem_page[PMM_PAGE_MAX_SIZE];

// 内存管理结构体
typedef struct pmm_manage {
    // 管理算法的名称
    const char *name;
    // 初始化
    void (*pmm_manage_init)(uint32_t pages);
    // 申请物理内存，单位为 Byte
    addr_t (*pmm_manage_alloc)(uint32_t bytes);
    // 释放内存页
    void (*pmm_manage_free)(addr_t addr_start, uint32_t bytes);
    // 返回当前可用内存页数量
    uint32_t (*pmm_manage_free_pages_count)(void);
} pmm_manage_t;

// 从 GRUB 读取物理内存信息
void pmm_get_ram_info(e820map_t *e820map);

// 物理内存管理初始化 参数为实际可用物理页数量
void pmm_mamage_init(uint32_t pages);

// 初始化内存管理
void pmm_init(void);

// 请求指定数量物理页
addr_t pmm_alloc_page(uint32_t pages);

// 释放内存页
void pmm_free_page(addr_t addr, uint32_t pages);

// 获取空闲内存页数量
uint32_t pmm_free_pages_count(void);

#ifdef __cplusplus
}
#endif

#endif /* _PMM_H_ */


// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// pmm.h for SimpleXX/SimpleKernel.

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
extern ptr_t *kernel_start;
extern ptr_t *kernel_text_start;
extern ptr_t *kernel_text_end;
extern ptr_t *kernel_data_start;
extern ptr_t *kernel_data_end;
extern ptr_t *kernel_end;

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
#define KERNEL_STACK_START (((ptr_t)(KERNEL_END_ADDR)) & PMM_PAGE_MASK)
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

// 总共3个区域
#define ZONE_SUM 3
// 内存分区对应下标
enum zone { DMA = 0, NORMAL = 1, HIGHMEM = 2 };

// 0
#define DMA_START_ADDR (0x0UL)
// 16MB
#define NORMAL_START_ADDR (0x1000000UL)
// 896MB
#define HIGHMEM_START_ADDR (0x38000000UL)
// 16MB
#define DMA_SIZE (0x1000000UL)
// 880MB
#define NORMAL_SIZE (0x37000000UL)
// 1152MB
#define HIGHMEM_SIZE (0x48000000UL)

// 物理页数量 131072,
// 0x20000，除去外设映射，实际可用物理页数量为 159 + 130800 =
// 130959 (这是物理内存为 512MB 的情况)
#define PMM_PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)

extern multiboot_memory_map_entry_t *mmap_entries;
extern multiboot_mmap_tag_t *        mmap_tag;

/*
 内存分区管理
 将物理内存地址 0-16MB 划分为 zone_DMA 区域
 将物理内存地址 16MB-110MB 划分为 zone_NORMAL 区域
 (按照 linux 内核 zone 机制应为 16MB-896 MB，但我们分配给 bochs 的内存只有
 512MB，所以此处按比例缩小） 将物理内存地址 110MB-结束（该内核大小为
 512MB）的区域划分为 zone_HIGHMEM 主要为了解决两个问题：
 1、某些 ISA 设备只能使用物理内存的前 16MB
 2、当物理内存过大时（超过 4GB），寻址空间不够用，则需通过映射实现。
*/
// 内存分区管理结构体
typedef struct memory_zone_mamage {
    // 该分区中空闲页的总数
    uint32_t free_pages;
    // 管理区极值，用于清理管理区
    uint32_t pages_min;
    uint32_t pages_low;
    uint32_t pages_high;
    //标志管理区是否应该进行清理，即可用页面是否达到管理区的一个极值
    bool need_balance;
    // 管理区总页数
    uint32_t all_pages;
} memory_zone_mamage_t;

// 物理页结构体
typedef struct physical_page {
    // 该页对应的内存分区
    int8_t zone;
    // 起始地址
    ptr_t start;
    // 该页被引用次数，-1代表外设映射区域，OS无法进行操作
    int32_t ref;
} physical_page_t;

// 分区数组
extern memory_zone_mamage_t mem_zone[ZONE_SUM];

// 可用内存的物理页数组
extern physical_page_t mem_page[PMM_PAGE_MAX_SIZE];

// 内存管理结构体
typedef struct pmm_manage {
    // 管理算法的名称
    const char *name;
    // 初始化
    void (*pmm_manage_init)();
    // 申请物理内存，单位为 Byte
    ptr_t (*pmm_manage_alloc)(uint32_t bytes, int8_t zone);
    // 释放内存页
    void (*pmm_manage_free)(ptr_t addr_start, uint32_t bytes, int8_t zone);
    // 返回当前可用内存页数量
    uint32_t (*pmm_manage_free_pages_count)(int8_t zone);
} pmm_manage_t;

// 从 GRUB 读取物理内存信息
void pmm_get_ram_info(e820map_t *e820map);

// 物理内存 zone 初始化
void pmm_zone_init(e820map_t *e820map);

// 物理内存管理初始化
void pmm_mamage_init();

// 初始化内存管理
void pmm_init(void);

// 请求 zone 区域的指定大小物理内存
ptr_t pmm_alloc(size_t byte, int8_t zone);

// 请求 zone 区域的指定数量物理页
ptr_t pmm_alloc_page(uint32_t pages, int8_t zone);

// 释放内存
void pmm_free_page(ptr_t addr, uint32_t byte, int8_t zone);

// 释放内存页
void pmm_free(ptr_t addr, uint32_t byte, int8_t zone);

// 获取指定 zone 空闲内存页数量
uint32_t pmm_free_pages_count(int8_t zone);

#ifdef __cplusplus
}
#endif

#endif /* _PMM_H_ */

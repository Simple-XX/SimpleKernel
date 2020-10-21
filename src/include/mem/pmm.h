
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
#include "e820.h"
#include "multiboot2.h"

// 8KB
#define KERNEL_STACK_SIZE (0x2000UL)
#define KERNEL_STACK_PAGES (KERNEL_STACK_SIZE / PMM_PAGE_SIZE)
#define KERNEL_STACK_BOTTOM (0xC0000000UL)
#define KERNEL_STACK_TOP (KERNEL_STACK_BOTTOM - KERNEL_STACK_SIZE)
// 512 MB
#define PMM_MAX_SIZE (0x20000000UL)

// 内核的偏移地址
#define KERNEL_BASE (0xC0000000UL)
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
/*************************/
//总共3个区域
#define zone_sum 3
//内存分区对应下标
#define DMA 0
#define NORMAL 1
#define HIGHMEM 2
#define DMA_start_addr (0x0)           // 0
#define NORMAL_start_addr (0x1000000)  // 16MB
#define HIGHMEM_start_addr (0x6e00000) // 110MB
#define DMA_SIZE (0x1000000)           // 16MB
#define NORMAL_SIZE (0x5e00000)        // 94MB
#define HIGHMEM_SIZE (0x19200000)      // 402MB

/*************************/
/*
更正版本：由于分配512MB内存，所以可用内存不会超过131072个物理页，除去外设映射，则可用内存段的物理页数量159+130800=130959
*/
#define PMM_PAGE_MAX_SIZE                                                      \
    (PMM_MAX_SIZE / PMM_PAGE_SIZE) // 物理页数量 131072, 0x20000

// A common problem is getting garbage data when trying to use a value defined
// in a linker script. This is usually because they're dereferencing the symbol.
// A symbol defined in a linker script (e.g. _ebss = .;) is only a symbol, not a
// variable. If you access the symbol using extern uint32_t _ebss; and then try
// to use _ebss the code will try to read a 32-bit integer from the address
// indicated by _ebss. The solution to this is to take the address of _ebss
// either by using it as & _ebss or by defining it as an unsized array(extern
// char _ebss[]; ) and casting to an integer.(The array notation prevents
// accidental reads from _ebss as arrays must be explicitly dereferenced) ref:
// http://wiki.osdev.org/Using_Linker_Script_Values
extern ptr_t *kernel_start;
extern ptr_t *kernel_text_start;
extern ptr_t *kernel_text_end;
extern ptr_t *kernel_data_start;
extern ptr_t *kernel_data_end;
extern ptr_t *kernel_end;
// 开启分页机制之后的内核栈
extern uint8_t kernel_stack[KERNEL_STACK_SIZE];
// 内核栈的栈顶
extern ptr_t kernel_stack_top;

extern multiboot_memory_map_entry_t *mmap_entries;
extern multiboot_mmap_tag_t *        mmap_tag;

/*******************************************************************************/
/***************************
            内存分区管理
        将物理内存地址0-16MB划分为zone_DMA区域
    将物理内存地址16MB-110MB划分为zone_NORMAL区域
（按照linux内核zone机制应为16MB-896MB，但我们分配给bochs的内存只有512MB，所以此处按比例缩小）
        将物理内存地址110MB-结束（该内核大小为512MB）的区域划分为zone_HIGHMEM
        主要为了解决2个问题：1、某些ISA设备只能使用物理内存的前16MB
2、当物理内存过大时（超过4GB），寻址空间不够用，则需通过映射实现。
****************************/
//内存分区管理结构体
typedef struct zone {
    //该分区中空闲页的总数
    uint32_t free_pages;
    //管理区极值，用于清理管理区
    uint32_t pages_min, pages_low, pages_high;
    //标志管理区是否应该进行清理，即可用页面是否达到管理区的一个极值
    bool need_balance;
    //管理区总页数
    uint32_t all_pages;
} memory_zone;
//物理页结构体
typedef struct page {
    //该页对应的内存分区 0-zone_DMA 1-zone_NORMAL 2-zone_HIGHMEM
    char zone;
    //起始地址
    ptr_t start;
    //该页被引用次数，-1代表外设映射区域，OS无法进行操作
    int32_t ref;
} physical_page;

//分区数组
extern memory_zone mem_zone[zone_sum];
//可用内存的物理页数组
extern physical_page mem_page[PMM_PAGE_MAX_SIZE];
/*******************************************************************************/
// 内存管理结构体
typedef struct pmm_manage {
    // 管理算法的名称
    const char *name;
    // 初始化
    void (*pmm_manage_init)();
    // 申请物理内存，单位为 Byte
    ptr_t (*pmm_manage_alloc)(uint32_t bytes, char zone);
    // 释放内存页
    void (*pmm_manage_free)(ptr_t addr_start, uint32_t bytes, char zone);
    // 返回当前可用内存页数量
    uint32_t (*pmm_manage_free_pages_count)(char zone);
} pmm_manage_t;

// 物理内存初始化
void pmm_phy_init(e820map_t *e820map);
// 物理内存管理初始化
void pmm_mamage_init();
// 初始化内存管理
void pmm_init(void);
// 缓冲区初始化
void  buffer_init();
ptr_t pmm_alloc(size_t byte, char zone);

void pmm_free_page(ptr_t addr, uint32_t byte, char zone);

uint32_t pmm_free_pages_count(char zone);

#ifdef __cplusplus
}
#endif

#endif /* _PMM_H_ */

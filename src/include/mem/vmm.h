
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.h for MRNIU/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "intr/include/intr.h"
#include "pmm.h"

// 内核的偏移地址
#define PAGE_OFFSET    0xC0000000

// 最大内存, 512MB, 0x20000000
#define KMEM_SIZE    PMM_MAX_SIZE

// 每个页表可以映射的内存数, 4MB
#define PAGE_MAP_SIZE    (0x400000)

// 映射 KMEM_SIZE 的内存所需要的页数 0x80, 128
#define PTE_COUNT    (KMEM_SIZE / PAGE_MAP_SIZE)

// 虚拟分页大小 4096
#define PAGE_SIZE    0x1000

// 页掩码，用于 4KB 对齐
#define PAGE_MASK    0xFFFFF000

// 页目录成员数, 128, 0x80，取决于实际内存
#define PGD_SIZE    (PAGE_SIZE / sizeof(pte_t))

// 页表成员数, 128, 0x80
#define PTE_SIZE    (PAGE_SIZE / sizeof(ptr_t))

// P-- 位 0 是存在 (Present) 标志，用于指明表项对地址转换是否有效。
// P = 1 表示有效； P = 0 表示无效。
#define PAGE_PRESENT    0x00000001

// R/W-- 位 1 是读 / 写 (Read/Write) 标志。如果等于 1  表示页面可以被读、写或执行。
// 如果为 0  表示页面只读或可执行。
#define PAGE_WRITE    0x00000002

// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
// 如果为 1  那么运行在任何特权级上的程序都可以访问该页面。
// 如果为 0  那么页面只能被运行在超级用户特权级 (0,1 或 2)  的程序访问。

#define PAGE_USER    0x00000004

// 获取一个地址的页目录项，高 10 位
#define PGD_INDEX(x) (((x) >> 22) & 0x03FF)

// 获取一个地址的页表项，中间 10 位
#define PTE_INDEX(x) ( ( ( x ) >> 12 ) & 0x03FF )

// 获取一个地址的页內偏移，低 12 位
#define OFFSET_INDEX(x) ( ( x ) & 0x0FFF )

// 页目录
typedef ptr_t pgd_t;

// 页表
typedef ptr_t pte_t;

// 内核页目录区域
extern pgd_t pgd_kernel[PGD_SIZE];

typedef
        struct page {
	uint32_t present    : 1;  // Page present in memory
	uint32_t rw         : 1;  // Read-only if clear, readwrite if set
	uint32_t user       : 1;  // Supervisor level only if clear
	uint32_t accessed   : 1;  // Has the page been accessed since last refresh?
	uint32_t dirty      : 1;  // Has the page been written to since last refresh?
	uint32_t unused     : 7;  // Amalgamation of unused and reserved bits
	uint32_t pte        : 10; // (((x)>>12) & 0x03FF ) middle 10 bits, PTE
	uint32_t pgd        : 10; // (((x) >> 22) & 0x03FF) high 10 bits, PGD
} page_t;

typedef
        struct page_table {
	page_t pages[1024];
} page_table_t;

typedef
        struct page_directory {
	// Array of pointers to pagetables.
	page_table_t *tables[1024];

	// Array of pointers to the pagetables above, but gives their *physical*
	// location, for loading into the CR3 register.
	ptr_t tablesPhysical[1024];

	// The physical address of tablesPhysical. This comes into play
	// when we get our kernel heap allocated and the directory
	// may be in a different location in virtual memory.
	ptr_t physicalAddr;
} page_directory_t;

// 初始化虚拟内存管理
void vmm_init(void);

// 使用 flags 指出的页权限，把物理地址 pa 映射到虚拟地址 va
void map(pgd_t *pgd_now, ptr_t va, ptr_t pa, uint32_t flags);

// 取消虚拟地址 va 的物理映射
void unmap(pgd_t *pgd_now, ptr_t va);

// 如果虚拟地址 va 映射到物理地址则返回 1
// 同时如果 pa 不是空指针则把物理地址写入 pa 参数
uint32_t get_mapping(pgd_t *pgd_now, ptr_t va, ptr_t pa);

// 更换当前页目录
void switch_pgd(ptr_t pd);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_VMM_H */

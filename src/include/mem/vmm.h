
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.h for MRNIU/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "intr/include/intr.h"
#include "vmm.h"

// 内核的偏移地址
#define VMM_PAGE_OFFSET    PMM_PAGE_OFFSET

// 页掩码，用于 4KB 对齐
#define VMM_PAGE_MASK    PMM_PAGE_MASK

// 页大小
#define VMM_PAGE_SIZE    PMM_PAGE_SIZE

// 页目录项大小
#define VMM_PGD_SIZE    (sizeof(pte_t) )

// 页表项大小
#define VMM_PTE_SIZE    (sizeof(ptr_t) )

#ifdef __x86_64__
// 虚拟地址位
#define VMM_VA_BITS     64
// 最大虚拟地址
#define VMEM_SIZE       (1 << VMM_VA_BITS)
#else
// 二级索引
// 虚拟地址位数
#define VMM_VA_BITS     32
// 最大虚拟地址, 4GB
#define VMEM_SIZE       (1 << VMM_VA_BITS)
// 页内偏移位数, log2(VMM_PAGE_SIZE)
#define VMM_PG_OFFSET_BITS      (12)
// 页数量
#define VMM_PAGE_COUNT      (1 << VMM_PG_OFFSET_BITS)
// 每页可以放入多少页表项
#define VMM_PTE_PRE_PAGE    (VMM_PAGE_SIZE / VMM_PTE_SIZE)
// 页表位数, log2(VMM_PTE_PRE_PAGE)
#define VMM_PTE_BITS    (10)
// 页表数量
#define VMM_PTE_COUNT   (1 << VMM_PTE_BITS)
// 每页可以放入多少页目录项
#define VMM_PGD_PRE_PAGE    (VMM_PAGE_SIZE / VMM_PGD_SIZE)
// 页目录位数, log2(VMM_PGD_PRE_PAGE)
#define VMM_PGD_BITS    (10)
// 页目录数量
#define VMM_PGD_COUNT   (1 << VMM_PGD_BITS)
#endif

// 虚拟页数量
// 0x100000000/0x1000 = 0x100000(32bits without PSE and PAE)
#define VMM_PAGE_COUNT      (VMEM_SIZE / VMM_PAGE_SIZE)

// P-- 位 0 是存在 (Present) 标志，用于指明表项对地址转换是否有效。
// P = 1 表示有效； P = 0 表示无效。
#define VMM_PAGE_PRESENT    0x00000001

// R/W-- 位 1 是读 / 写 (Read/Write) 标志。如果等于 1  表示页面可以被读、写或执行。
// 如果为 0  表示页面只读或可执行。
#define VMM_PAGE_WRITE    0x00000002

// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
// 如果为 1  那么运行在任何特权级上的程序都可以访问该页面。
// 如果为 0  那么页面只能被运行在超级用户特权级 (0,1 或 2)  的程序访问。
#define VMM_PAGE_USER    0x00000004

// 获取一个地址的页目录，高 10 位
#define VMM_PGD_INDEX(x) ( ( (x) >> 22) & 0x03FF)

// 获取一个地址的页表，中间 10 位
#define VMM_PTE_INDEX(x) ( ( (x) >> 12) & 0x03FF)

// 获取一个地址的页內偏移，低 12 位
#define VMM_OFFSET_INDEX(x) ( (x) & 0x0FFF)

// 映射内核页需要的页数 0x40000
// #define VMM_KERNEL_PAGE_COUNT   (0x40000000/VMM_PAGE_SIZE)
#define VMM_KERNEL_PAGE_COUNT   (0x20000000 / VMM_PAGE_SIZE)

// 页全局目录
typedef ptr_t pgd_t;

// 页上级目录
typedef ptr_t pmd_t;

// 页中间目录
typedef ptr_t pud_t;

// 页表
typedef ptr_t pte_t;

typedef
        struct page {
	// must be 1 to map a 4-KByte page
	uint32_t present    : 1;
	// if 0, writes may not be allowed to the 4-KByte page referenced by this entry
	uint32_t rw         : 1;
	// if 0, user-mode accesses are not allowed to the 4-KByte page referenced by this entry
	uint32_t user       : 1;
	//  indirectly determines the memory type used to access the 4-KByte page referenced by this entry
	uint32_t pwt        : 1;
	// indirectly determines the memory type used to access the 4-KByte page referenced by this entry
	uint32_t pcd        : 1;
	// indicates whether software has accessed the 4-KByte page referenced by this entry
	uint32_t accessed   : 1;
	// indicates whether software has written to the 4-KByte page referenced by this entry
	uint32_t dirty      : 1;
	// If the PAT is supported, indirectly determines the memory type used to access
	// the 4-KByte page referenced by this entry (see Section 4.9.2); otherwise, reserved (must be 0)1
	uint32_t pat        : 1;
	// Global; if CR4.PGE = 1, determines whether the translation is global (see Section 4.10); ignored otherwise
	uint32_t global     : 1;
	// ignore
	uint32_t ignore     : 3;
	// Physical address of the 4-KByte page referenced by this entry
	uint32_t addr       : 20;
} page_t;

typedef
        struct page_table {
	// must be 1 to reference a page table
	uint32_t present    : 1;
	// if 0, writes may not be allowed to the 4-MByte region controlled by this entry
	uint32_t rw         : 1;
	//  if 0, user-mode accesses are not allowed to the 4-MByte region controlled by this entry
	uint32_t user       : 1;
	//  indirectly determines the memory type used to access the page table referenced by this entry
	uint32_t pwt        : 1;
	// indirectly determines the memory type used to access the page table referenced by this entry
	uint32_t pcd        : 1;
	// indicates whether this entry has been used for linear-address translation
	uint32_t accessed   : 1;
	// ignore
	uint32_t ignore1    : 1;
	// If CR4.PSE = 1, must be 0 (otherwise, this entry maps a 4-MByte page; see Table 4-4); otherwise, ignored
	uint32_t ps         : 1;
	// ignore
	uint32_t ignore2    : 4;
	// Physical address of 4-KByte aligned page table referenced by this entry
	uint32_t addr       : 20;
} page_table_t;

typedef
        struct page_directory {
	page_table_t page_table[VMM_PTE_COUNT];
} page_directory_t;

// 初始化虚拟内存管理
void vmm_init(void);

// 使用 flags 指出的页权限，把物理地址 pa 映射到虚拟地址 va
void map(pgd_t * pgd_now, ptr_t va, ptr_t pa, uint32_t flags);

// 取消虚拟地址 va 的物理映射
void unmap(pgd_t * pgd_now, ptr_t va);

// 如果虚拟地址 va 映射到物理地址则返回 1
// 同时如果 pa 不是空指针则把物理地址写入 pa 参数
uint32_t get_mapping(pgd_t * pgd_now, ptr_t va, ptr_t pa);

// 更换当前页目录
void switch_pgd(ptr_t pd);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_VMM_H */


// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.h for MRNIU/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pmm.h"

// 页大小，一页能映射多少 Byte 内存 2^12
#define VMM_PAGE_SIZE       (PMM_PAGE_SIZE)
// 页掩码
#define VMM_PAGE_MASK       (PMM_PAGE_MASK)
// 页表项 sizeof(pte_t) 大小 = 4B 2^2
#define VMM_PTE_SIZE        (sizeof(pte_t) )
// 每页能映射多少页表项 = 页大小/页表项大小 2^10
#define VMM_PTE_PRE_PAGE    (VMM_PAGE_SIZE / VMM_PTE_SIZE)
// 页目录项 sizeof(pgd_t) 大小 = 4B 2^2
#define VMM_PGDE_SIZE       (sizeof(pgd_t) )
// 每页能映射多少页目录项 = 页大小/页目录项大小 2^10
#define VMM_PGDE_PRE_PAGE   (VMM_PAGE_SIZE / VMM_PGDE_SIZE)
// 每个页表能映射多少页 = 页大小/页表项大小 2^10
#define VMM_PAGES_PRE_PAGE_TABLE            (VMM_PAGE_SIZE / VMM_PTE_SIZE)
// 每个页目录能映射多少页表 = 页大小/页目录项大小 2^10
#define VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY  (VMM_PAGE_SIZE / VMM_PGDE_SIZE)
// 每个页目录能映射多少页 = 页表数量*每个页表能映射多少页 2^20
#define VMM_PAGES_PRE_PAGE_DIRECTORY        (VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY * VMM_PAGES_PRE_PAGE_TABLE)
// 页表大小，一页表能映射多少 Byte 内存 = 页表项数量*页表项映射大小 2^22
#define VMM_PAGE_TABLE_SIZE (VMM_PAGES_PRE_PAGE_TABLE * VMM_PAGE_SIZE)
// 页目录大小，一页目录能映射多少 Byte 内存 = 页表数量*页表映射大小 2^32
#define VMM_PAGE_DIRECTORY_SIZE     ( (uint64_t)VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY * (uint64_t)VMM_PAGE_TABLE_SIZE)
// 虚拟内存位数
#define VMM_VMEM_BITS       (32UL)
// 虚拟内存大小
#define VMM_VMEM_SIZE       (1UL << (VMM_VMEM_BITS - 1UL) )
// 映射全部虚拟内存需要的页数 = 虚拟内存大小/页大小 2^20
#define VMM_PAGES_TOTAL     (VMM_VMEM_SIZE / VMM_PAGE_SIZE)
// 映射全部虚拟内存需要的页表数 = 虚拟内存大小/页表大小 2^12
#define VMM_PAGE_TABLES_TOTAL       (VMM_VMEM_SIZE / VMM_PAGE_TABLE_SIZE)
// 映射全部虚拟内存需要的页目录数 = 虚拟内存大小/页目录大小 2^0
#define VMM_PAGE_DIRECTORIES_TOTAL  (VMM_VMEM_SIZE / VMM_PAGE_DIRECTORY_SIZE)
// 映射内核需要的页数
#define VMM_PAGES_KERNEL    (PMM_PAGES_KERNEL)
// 映射内核需要的页表数
#define VMM_PAGE_TABLES_KERNEL      ( (KERNEL_SIZE / VMM_PAGE_TABLE_SIZE) + 1UL)
// 映射内核需要的页目录数
#define VMM_PAGE_DIRECTORIES_KERNEL      ( (KERNEL_SIZE / VMM_PAGE_DIRECTORY_SIZE) + 1UL)

// P = 1 表示有效； P = 0 表示无效。
#define VMM_PAGE_PRESENT    (0x00000001)

// 如果为 0  表示页面只读或可执行。
#define VMM_PAGE_RW      (0x00000002)

// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
// 如果为 1  那么运行在任何特权级上的程序都可以访问该页面。
#define VMM_PAGE_USER       (0x00000004)
// 如果为 0  那么页面只能被运行在超级用户特权级 (0,1 或 2)  的程序访问。
#define VMM_PAGE_KERNEL     (0x00000000)

// 获取一个地址的页目录，高 10 位
#define VMM_PGD_INDEX(x)        ( ( (x) >> 22) & 0x03FF)

// 获取一个地址的页表，中间 10 位
#define VMM_PTE_INDEX(x)        ( ( (x) >> 12) & 0x03FF)

// 获取一个地址的页內偏移，低 12 位
#define VMM_OFFSET_INDEX(x)     ( (x) & 0x0FFF)

// 页全局目录项
typedef ptr_t pgd_t;

// 页上级目录项，以后拓展的时候用
typedef ptr_t pmd_t;

// 页中间目录项，以后拓展的时候用
typedef ptr_t pud_t;

// 页表项
typedef ptr_t pte_t;

// 内核页目录区域
extern pgd_t pgd_kernel[VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

// 初始化虚拟内存管理
void vmm_init(void);

// 使用 flags 指出的页权限，把物理地址 pa 映射到虚拟地址 va
void map(pgd_t * pgd_now, ptr_t va, ptr_t pa, uint32_t flags);

// 取消虚拟地址 va 的物理映射
void unmap(pgd_t * pgd_now, ptr_t va);

// 如果虚拟地址 va 映射到物理地址则返回 1
// 同时如果 pa 不是空指针则把物理地址写入 pa 参数
uint32_t get_mapping(pgd_t * pgd_now, ptr_t va, ptr_t * pa);

// 更换当前页目录
void switch_pgd(ptr_t pd);

// 初始化内核页目录
void vmm_kernel_init(pgd_t * pgd);

#define VMM_LA_PA(la) (la - KERNEL_BASE)
#define VMM_PA_LA(pa) (pa + KERNEL_BASE)

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_VMM_H */

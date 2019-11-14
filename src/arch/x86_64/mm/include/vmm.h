
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// vmm.h for MRNIU/SimpleKernel.

#ifndef _HEAP_H_
#define _HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "intr/include/intr.h"
#include "mm/include/pmm.h"

// 每个页表可以映射的内存数
#define PAGE_MAP_SIZE    ( 0x400000 )

// 映射 KMEM_SIZE 的内存所需要的页数
#define PTE_COUNT         ( KMEMSIZE / PAGE_MAP_SIZE )

// 内核的偏移地址
#define PAGE_OFFSET     0xC0000000

/**
 * P-- 位 0 是存在 (Present) 标志，用于指明表项对地址转换是否有效。
 * P = 1 表示有效； P = 0 表示无效。
 * 在页转换过程中，如果说涉及的页目录或页表的表项无效，则会导致一个异常。
 * 如果 P = 0 ，那么除表示表项无效外，其余位可供程序自由使用。
 * 例如，操作系统可以使用这些位来保存已存储在磁盘上的页面的序号。
 */
#define PAGE_PRESENT    0x1

/**
 * R/W -- 位 1 是读 / 写 (Read/Write) 标志。如果等于 1 ，表示页面可以被读、写或执行。
 * 如果为 0 ，表示页面只读或可执行。
 * 当处理器运行在超级用户特权级 （级别 0,1 或 2） 时，则 R/W 位不起作用。
 * 页目录项中的 R/W 位对其所映射的所有页面起作用。
 */
#define PAGE_WRITE  0x2

/**
 * U/S -- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
 * 如果为 1 ，那么运行在任何特权级上的程序都可以访问该页面。
 * 如果为 0 ，那么页面只能被运行在超级用户特权级 (0,1 或 2) 上的程序访问。
 * 页目录项中的 U/S 位对其所映射的所有页面起作用。
 */
#define PAGE_USER   0x4

// 虚拟分页大小
#define PAGE_SIZE   4096

// 页掩码，用于 4KB 对齐
#define PAGE_MASK      0xFFFFF000

// 获取一个地址的页目录项
#define PGD_INDEX(x) ( ( ( x ) >> 22 ) & 0x03FF )

// 获取一个地址的页表项
#define PTE_INDEX(x) ( ( ( x ) >> 12 ) & 0x03FF )

// 获取一个地址的页內偏移
#define OFFSET_INDEX(x) ( ( x ) & 0x0FFF )

// 页目录
typedef uint64_t pgd_t;

// 页表
typedef uint64_t pte_t;

// 页目录成员数
#define PGD_SIZE ( PAGE_SIZE / sizeof( pte_t ) )

// 页表成员数
#define PTE_SIZE ( PAGE_SIZE / sizeof( uint64_t ) )

// 映射 512MB 内存所需要的页表数
#define PTE_COUNT 128

// 内核页目录区域
extern pgd_t pgd_kernel[PGD_SIZE];

// 初始化虚拟内存管理
void vmm_init(void);

// 更换当前的页目录
void switch_pgd(uint32_t pd);

// 使用 flags 指出的页权限，把物理地址 pa 映射到虚拟地址 va
void map(pgd_t *pgd_low, uint32_t va, uint32_t pa, uint32_t flags);

// 取消虚拟地址 va 的物理映射
void unmap(pgd_t *pgd_low, uint32_t va);

// 如果虚拟地址 va 映射到物理地址则返回 1
// 同时如果 pa 不是空指针则把物理地址写入 pa 参数
uint32_t get_mapping(pgd_t *pgd_low, uint32_t va, uint32_t *pa);

// 页错误中断的函数处理
void page_fault(pt_regs_t *regs);

void vmm_init(void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif  // INCLUDE_VMM_H

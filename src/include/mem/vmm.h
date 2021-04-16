
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.h for Simple-XX/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#include "limits.h"
#include "common.h"
#include "pmm.h"
#include "io.h"

typedef void *              page_t;
typedef page_t              page_table_entry_t;
typedef page_table_entry_t *page_table_t;
typedef page_table_t        page_dir_entry_t;
typedef page_dir_entry_t *  page_dir_t;

// 根据不同情况有区别
// 一个页下级目录有多少个页表项
static constexpr const uint32_t PTES_PRE_PLD =
    sizeof(void *) / COMMON::PAGE_SIZE;
// 一个页中级目录有多少个页下级目录
static constexpr const uint32_t PLDS_PRE_PMD = 1;
// 一个页上级目录有多少个页中级目录
static constexpr const uint32_t PMDS_PRE_PUD = 1;
// 一个页全局目录有多少个页上级目录
static constexpr const uint32_t PUDS_PRE_PGD = 1024;

// 页表项
using pte_t = void *;
// 页下级目录
using pld_t = pte_t[PTES_PRE_PLD];
// 页中级目录
using pmd_t = pld_t[PLDS_PRE_PMD];
// 页上级目录
using pud_t = pmd_t[PMDS_PRE_PUD];
// 页全局目录
using pgd_t = pud_t[PUDS_PRE_PGD];

// 页表项 sizeof(pte_t) 大小 = 4B 2^3
static constexpr const uint32_t VMM_PTE_SIZE = sizeof(page_table_entry_t);

// 每页能映射多少页表项 = 页大小/页表项大小 2^9
static constexpr const uint32_t VMM_PTE_PRE_PAGE =
    COMMON::PAGE_SIZE / VMM_PTE_SIZE;

// 页目录项 sizeof(pgd_t) 大小 = 4B 2^3
static constexpr const uint32_t VMM_PGDE_SIZE = sizeof(page_dir_entry_t);

// 每页能映射多少页目录项 = 页大小/页目录项大小 2^9
static constexpr const uint32_t VMM_PGDE_PRE_PAGE =
    COMMON::PAGE_SIZE / VMM_PGDE_SIZE;

// 每个页表能映射多少页 = 页大小/页表项大小: 2^9
static constexpr const uint32_t VMM_PAGES_PRE_PAGE_TABLE =
    COMMON::PAGE_SIZE / VMM_PTE_SIZE;

// 每个页目录能映射多少页表 = 页大小/页目录项大小 2^9
static constexpr const uint32_t VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY =
    COMMON::PAGE_SIZE / VMM_PGDE_SIZE;

// 每个页目录能映射多少页 = 页表数量*每个页表能映射多少页 2^18
static constexpr const uint32_t VMM_PAGES_PRE_PAGE_DIRECTORY =
    VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY * VMM_PAGES_PRE_PAGE_TABLE;

// 页表大小，一页表能映射多少 Byte 内存 = 页表项数量*页表项映射大小 2^21
static constexpr const uint64_t VMM_PAGE_TABLE_SIZE =
    VMM_PAGES_PRE_PAGE_TABLE * COMMON::PAGE_SIZE;

// 页目录大小，一页目录能映射多少 Byte 内存 = 页表数量*页表映射大小 2^32
static constexpr const uint64_t VMM_PAGE_DIRECTORY_SIZE =
    VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY * VMM_PAGE_TABLE_SIZE;

// 虚拟内存大小 4GB
static constexpr const uint64_t VMM_VMEM_SIZE = UINT_MAX + (uint64_t)1;

// 映射全部虚拟内存需要的页数 = 虚拟内存大小/页大小 2^20
static constexpr const uint64_t VMM_PAGES_TOTAL =
    VMM_VMEM_SIZE / COMMON::PAGE_SIZE;

// 映射全部虚拟内存需要的页表数 = 虚拟内存大小/页表大小 2^12
static constexpr const uint64_t VMM_PAGE_TABLES_TOTAL =
    VMM_VMEM_SIZE / VMM_PAGE_TABLE_SIZE;

// 映射全部虚拟内存需要的页目录数 = 虚拟内存大小/页目录大小 2^0
static constexpr const uint64_t VMM_PAGE_DIRECTORIES_TOTAL =
    (VMM_VMEM_SIZE / VMM_PAGE_DIRECTORY_SIZE) +
    ((VMM_VMEM_SIZE % VMM_PAGE_DIRECTORY_SIZE) == 0 ? 0 : 1);

// 映射内核需要的页表数
static constexpr const uint64_t VMM_PAGE_TABLES_KERNEL =
    (COMMON::KERNEL_SIZE / VMM_PAGE_TABLE_SIZE) +
    (COMMON::KERNEL_SIZE % VMM_PAGE_TABLE_SIZE == 0 ? 0 : 1);

// 映射内核的大小
static constexpr const uint64_t VMM_KERNEL_SIZE = COMMON::KERNEL_SIZE;

// 内核映射的页数
static constexpr const uint64_t VMM_KERNEL_PAGES =
    VMM_KERNEL_SIZE / COMMON::PAGE_SIZE;

// 内核映射的页表数
static constexpr const uint64_t VMM_KERNEL_PAGE_TABLES =
    VMM_KERNEL_SIZE / VMM_PAGE_TABLE_SIZE;

// 内核映射的页目录数
static constexpr const uint64_t VMM_KERNEL_PAGE_DIRECTORIES =
    (VMM_KERNEL_SIZE / VMM_PAGE_DIRECTORY_SIZE) +
    ((VMM_KERNEL_SIZE % VMM_PAGE_DIRECTORY_SIZE) == 0 ? 0 : 1);

#if defined(__i386__) || defined(__x86_64__)
// 如果为 0  那么页面只能被运行在超级用户特权级 (0,1 或 2)  的程序访问。
static constexpr const uint32_t VMM_PAGE_KERNEL = 0x00000000;
// P = 1 表示有效； P = 0 表示无效。
static constexpr const uint32_t VMM_PAGE_PRESENT = 0x00000001;
// 如果为 0  表示页面只读或可执行。
static constexpr const uint32_t VMM_PAGE_RW = 0x00000002;
// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
// 如果为 1  那么运行在任何特权级上的程序都可以访问该页面。
static constexpr const uint32_t VMM_PAGE_USER = 0x00000004;

#elif defined(__riscv)
/// 有效位
static constexpr const uint8_t VMM_PAGE_VALID = 1 << 0;
/// 可读位
static constexpr const uint8_t VMM_PAGE_READABLE = 1 << 1;
/// 可写位
static constexpr const uint8_t VMM_PAGE_WRITABLE = 1 << 2;
/// 可执行位
static constexpr const uint8_t VMM_PAGE_EXECUTABLE = 1 << 3;
/// 用户位
static constexpr const uint8_t VMM_PAGE_USER = 1 << 4;
/// 全局位，我们不会使用
static constexpr const uint8_t VMM_PAGE_GLOBAL = 1 << 5;
/// 已使用位，用于替换算法
static constexpr const uint8_t VMM_PAGE_ACCESSED = 1 << 6;
/// 已修改位，用于替换算法
static constexpr const uint8_t VMM_PAGE_DIRTY = 1 << 7;
#endif

// 逻辑地址到物理地址转换
#define VMM_LA_PA(la) (la - COMMON::KERNEL_BASE)

// 物理地址到逻辑地址转换
#define VMM_PA_LA(pa) (pa + COMMON::KERNEL_BASE)

class VMM {
private:
    static IO  io;
    static PMM pmm;
    // 页表
    static page_table_t pte_kernel[VMM_KERNEL_PAGES]
        __attribute__((aligned(COMMON::PAGE_SIZE)));
    // 页目录
    static page_dir_t pgd_kernel[VMM_PAGE_TABLES_TOTAL]
        __attribute__((aligned(COMMON::PAGE_SIZE)));

    // static pgd_t pgd_kernel __attribute__((aligned(COMMON::PAGE_SIZE)));
    page_dir_t curr_dir;

protected:
public:
    VMM(void);
    ~VMM(void);
    // 初始化
    void init(void);
    // 获取当前页目录
    page_dir_t get_pgd(void) const;
    // 设置当前页目录
    void set_pgd(const page_dir_t pgd);
    // 映射物理地址到虚拟地址
    void mmap(const page_dir_t pgd, const void *va, const void *pa,
              const uint32_t flag);
    // 取消映射
    void unmmap(const page_dir_t pgd, const void *va);
    // 获取映射的物理地址
    // 已映射返回 1，未映射返回 0
    uint32_t get_mmap(const page_dir_t pgd, const void *va, const void *pa);
};

#endif /* _VMM_H */

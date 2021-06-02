
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.h for Simple-XX/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#include "limits.h"
#include "common.h"

typedef ptrdiff_t  pte_t;
typedef ptrdiff_t *pt_t;
typedef ptrdiff_t *pmd_t;
typedef ptrdiff_t *pud_t;
typedef ptrdiff_t *pgd_t;

// 每个页表能映射多少页 = 页大小/页表项大小: 2^9
static constexpr const uint32_t VMM_PAGES_PRE_PAGE_TABLE =
    COMMON::PAGE_SIZE / sizeof(pte_t);

// 映射内核的大小
static constexpr const uint64_t VMM_KERNEL_SIZE = COMMON::KERNEL_SIZE;

// 内核映射的页数
static constexpr const uint64_t VMM_KERNEL_PAGES =
    VMM_KERNEL_SIZE / COMMON::PAGE_SIZE;

#if defined(__i386__) || defined(__x86_64__)
// P = 1 表示有效； P = 0 表示无效。
static constexpr const uint32_t VMM_PAGE_VALID = 1 << 0;
// 如果为 0  表示页面只读或可执行。
static constexpr const uint32_t VMM_PAGE_READABLE   = 0;
static constexpr const uint32_t VMM_PAGE_WRITABLE   = 1 << 1;
static constexpr const uint32_t VMM_PAGE_EXECUTABLE = 0;
// U/S-- 位 2 是用户 / 超级用户 (User/Supervisor) 标志。
// 如果为 1 那么运行在任何特权级上的程序都可以访问该页面。
static constexpr const uint32_t VMM_PAGE_USER = 1 << 2;

#elif defined(__riscv)
// 有效位
static constexpr const uint8_t VMM_PAGE_VALID = 1 << 0;
// 可读位
static constexpr const uint8_t VMM_PAGE_READABLE = 1 << 1;
// 可写位
static constexpr const uint8_t VMM_PAGE_WRITABLE = 1 << 2;
// 可执行位
static constexpr const uint8_t VMM_PAGE_EXECUTABLE = 1 << 3;
// 用户位
static constexpr const uint8_t VMM_PAGE_USER = 1 << 4;
// 全局位，我们不会使用
static constexpr const uint8_t VMM_PAGE_GLOBAL = 1 << 5;
// 已使用位，用于替换算法
static constexpr const uint8_t VMM_PAGE_ACCESSED = 1 << 6;
// 已修改位，用于替换算法
static constexpr const uint8_t VMM_PAGE_DIRTY = 1 << 7;
#endif

// 逻辑地址到物理地址转换
#define VMM_LA_PA(la) (la - COMMON::KERNEL_BASE)

// 物理地址到逻辑地址转换
#define VMM_PA_LA(pa) (pa + COMMON::KERNEL_BASE)

class VMM {
private:
    // TODO: 支持最多四级页表，共用同一套代码
    // 当前页目录
    pgd_t curr_dir;

protected:
public:
    VMM(void);
    ~VMM(void);
    // 初始化
    int32_t init(void);
    // 获取当前页目录
    pgd_t get_pgd(void) const;
    // 设置当前页目录
    void set_pgd(const pgd_t pgd);
    // 映射物理地址到虚拟地址
    void mmap(const pgd_t pgd, const void *va, const void *pa,
              const uint32_t flag);
    // 取消映射
    void unmmap(const pgd_t pgd, const void *va);
    // 获取映射的物理地址
    // 已映射返回 1，未映射返回 0
    uint32_t get_mmap(const pgd_t pgd, const void *va, const void *pa);
};

extern VMM vmm;

#endif /* _VMM_H */

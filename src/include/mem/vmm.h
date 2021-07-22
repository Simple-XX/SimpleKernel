
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vmm.h for Simple-XX/SimpleKernel.

#ifndef _VMM_H_
#define _VMM_H_

#include "limits.h"
#include "common.h"

// TODO: 可以优化

// 页表项，最底层
typedef ptrdiff_t pte_t;
// 页表，也可以页目录，它们的结构是一样的
typedef ptrdiff_t *pt_t;
// 页目录，其实就是一种特殊的页表
typedef pt_t pgd_t;

// 每个页表能映射多少页 = 页大小/页表项大小: 2^9
static constexpr const uint32_t VMM_PAGES_PRE_PAGE_TABLE =
    COMMON::PAGE_SIZE / sizeof(pte_t);

// 映射内核空间的大小
static constexpr const uint64_t VMM_KERNEL_SPACE_SIZE =
    COMMON::KERNEL_SPACE_SIZE;

// 内核映射的页数
static constexpr const uint64_t VMM_KERNEL_SPACE_PAGES =
    VMM_KERNEL_SPACE_SIZE / COMMON::PAGE_SIZE;

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
// 内核虚拟地址相对物理地址的偏移
static constexpr const uint32_t KERNEL_OFFSET = 0x0;

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
// 内核虚拟地址相对物理地址的偏移
static constexpr const uint64_t KERNEL_OFFSET = 0x0;
#endif

// 虚拟地址到物理地址转换
static constexpr uintptr_t VMM_VA2PA(const void *_va) {
    return ((uintptr_t)_va) - KERNEL_OFFSET;
}

// 物理地址到虚拟地址转换
static constexpr uintptr_t VMM_PA2VA(const void *_pa) {
    return ((uintptr_t)_pa) + KERNEL_OFFSET;
}

class VMM {
private:
    // TODO: 支持最多四级页表，共用同一套代码
    // 当前页目录
    static pgd_t curr_dir;

protected:
public:
    VMM(void);
    ~VMM(void);
    // 初始化
    static bool init(void);
    // 获取当前页目录
    static pgd_t get_pgd(void);
    // 设置当前页目录
    static void set_pgd(const pgd_t _pgd);
    // 映射物理地址到虚拟地址
    static void mmap(const pgd_t _pgd, const void *_va, const void *_pa,
                     const uint32_t _flag);
    // 取消映射
    static void unmmap(const pgd_t _pgd, const void *_va);
    // 获取映射的物理地址
    // 已映射返回 true，未映射返回 false
    static bool get_mmap(const pgd_t _pgd, const void *_va, const void *_pa);
};

#endif /* _VMM_H */

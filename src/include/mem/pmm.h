
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.h for Simple-XX/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#include "stddef.h"
#include "stdint.h"
#include "firstfit.h"
#include "allocator.h"

// 对物理内存的管理来说
// 1. 管理所有的物理内存，不论是否被机器保留/无法访问
// 2. 内存开始地址与长度由 bootloader 给出: x86 下为 grub, riscv 下为 opensbi
// 3.
// 不关心内存是否被使用，但是默认的物理内存分配空间从内核结束后开始
// 如果由体系结构需要分配内核开始前内存空间的，则尽量避免
// 4. 最管理单位为页

// TODO:
// 目前的代码只能在全部内存中分配，如果需要分配到内核空间/用户空间就没办法了
// 两个分配器，管两个区域即可
class PMM {
public:
    // 保存物理地址信息
    struct phy_mem_t {
        void * addr;
        size_t len;
    };

private:
    static phy_mem_t phy_mem;

    // 物理内存开始地址
    static const void *start;
    // 物理内存长度，单位为 bytes
    static size_t length;
    // 物理内存页数
    static size_t total_pages;
    // 内核与非内核空间的地址与长度，单位为 bytes
    static const void *kernel_space_start;
    static size_t      kernel_space_length;
    static const void *non_kernel_space_start;
    static size_t      non_kernel_space_length;
    // 内核空间不会位于内存中间，导致出现非内核空间被切割为两部分的情况
    // 物理内存分配器，分配内核空间
    static ALLOCATOR *kernel_space_allocator;
    // 物理内存分配器，分配非内核空间
    static ALLOCATOR *allocator;
    // 将 multiboot2/dtb 信息移动到内核空间
    static void move_boot_info(void);

protected:
public:
    // 构造函数，目前为空
    // TODO: 从 bootloader 接受内存参数进行初始化
    PMM(void);
    ~PMM(void);
    // 初始化
    // TODO: 移动到构造函数去
    static bool init(void);
    // 获取物理内存长度
    static size_t get_pmm_length(void);
    // 分配一页
    static void *alloc_page(void);
    // 分配多页
    static void *alloc_pages(size_t _len);
    // 分配以指定地址开始的 _len 页
    // 如果此地址已使用，函数返回 true
    static bool alloc_pages(void *_addr, size_t _len);
    // 申请内核空间
    static void *alloc_page_kernel(void);
    static void *alloc_pages_kernel(size_t _len);
    static bool  alloc_pages_kernel(void *_addr, size_t _len);
    // 回收一页
    static void free_page(void *_addr);
    // 回收多页
    static void free_pages(void *_addr, size_t _len);
    // 获取当前已使用页数
    static uint64_t get_used_pages_count(void);
    // 获取当前空闲页
    static uint64_t get_free_pages_count(void);
};

#endif /* _PMM_H_ */

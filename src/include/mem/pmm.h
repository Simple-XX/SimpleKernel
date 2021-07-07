
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

class PMM {
private:
    // 物理内存开始地址
    const void *start;
    // 物理内存长度
    size_t length;
    // 物理内存页数
    static size_t total_pages;
    // 物理内存分配器
    static ALLOCATOR *allocator;

protected:
public:
    // 构造函数，目前为空
    // TODO: 从 bootloader 接受内存参数进行初始化
    PMM(void);
    ~PMM(void);
    // 初始化
    // TODO: 移动到构造函数去
    bool init(void);
    // 分配一页
    const void *alloc_page(void);
    // 分配多页
    const void *alloc_pages(size_t _len);
    // 分配以指定地址开始的 _len 页
    // 如果此地址已使用，函数返回 true
    bool alloc_pages(void *_addr, size_t _len);
    // 回收一页
    void free_page(const void *_addr);
    // 回收多页
    void free_pages(const void *_addr, size_t _len);
    // 获取当前已使用页数
    uint64_t get_used_pages_count(void) const;
    // 获取当前空闲页
    uint64_t get_free_pages_count(void) const;
};

extern PMM pmm;

#endif /* _PMM_H_ */

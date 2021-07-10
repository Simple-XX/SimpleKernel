
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// allocator.h for Simple-XX/SimpleKernel.

#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include "stddef.h"
#include "stdint.h"

// 内存分配器抽象
class ALLOCATOR {
private:
protected:
    // 分配器名称
    char *name;
    // 当前管理的内存区域地址
    void *addr;
    // 当前管理的内存区域长度
    size_t length;
    // 当前管理的内存区域页数
    size_t pages_count;
    // 当前管理的内存区域空闲页数
    size_t pages_free_count;
    // 当前管理的内存区域已使用页数
    size_t pages_used_count;

public:
    // _addr: 要管理的内存开始地址
    // _len: 要管理的内存长度，以页为单位
    ALLOCATOR(const void *_addr, size_t _len);
    virtual ~ALLOCATOR(void) = 0;
    // 分配 _len 页
    virtual void *alloc(size_t _len) = 0;
    // 在指定地址分配 _len 页
    // 如果此地址已使用，函数返回 true
    virtual bool alloc(void *_addr, size_t _len) = 0;
    // 释放一页
    virtual void free(void *_addr) = 0;
    // 已使用页数量
    virtual size_t get_used_pages_count(void) const = 0;
    // 空闲页数量
    virtual size_t get_free_pages_count(void) const = 0;
};

#endif /* _ALLOCATOR_H_ */

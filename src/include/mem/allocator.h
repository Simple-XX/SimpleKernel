
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
    const char *name;
    // 当前管理的内存区域地址
    uintptr_t allocator_start_addr;
    // 当前管理的内存区域长度
    size_t allocator_length;
    // 当前管理的内存区域空闲长度
    size_t allocator_free_count;
    // 当前管理的内存区域已使用长度
    size_t allocator_used_count;

public:
    // _name: 分配器名
    // _addr: 要管理的内存开始地址
    // _len: 要管理的内存长度，单位以具体实现为准
    ALLOCATOR(const char *_name, uintptr_t _addr, size_t _len);
    virtual ~ALLOCATOR(void) = 0;
    // 分配 _len 页
    virtual uintptr_t alloc(size_t _len) = 0;
    // 在指定地址分配 _len 长度
    // 如果此地址已使用，函数返回 false
    virtual bool alloc(uintptr_t _addr, size_t _len) = 0;
    // 释放 _len 长度
    virtual void free(uintptr_t _addr, size_t _len) = 0;
    // 已使用数量
    virtual size_t get_used_count(void) const = 0;
    // 空闲数量
    virtual size_t get_free_count(void) const = 0;
};

#endif /* _ALLOCATOR_H_ */

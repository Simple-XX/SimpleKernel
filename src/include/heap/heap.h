
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.h for Simple-XX/SimpleKernel.

#ifndef _HEAP_H_
#define _HEAP_H_

#include "stdint.h"
#include "stddef.h"
#include "slab.h"
#include "allocator.h"

// TODO: 添加对 VMM 的处理
class HEAP {
private:
    // 堆开始地址
    const void *start;
    // 堆长度 单位为 byte
    size_t length;
    // 堆页数
    size_t total_pages;
    // 堆分配器
    ALLOCATOR *allocator;

protected:
public:
    HEAP(void);
    ~HEAP(void);
    // 初始化
    bool init(const void *_addr, size_t _len);
    // 内存申请，单位为 Byte
    void *malloc(size_t _byte);
    // 内存释放
    void free(void *_p);
    // 获取管理的内存大小，包括管理信息
    size_t get_total(void);
    // 获取块数量
    size_t get_block(void);
    // 获取空闲内存数量 单位为 byte
    size_t get_free(void);
};

extern HEAP heap;

#endif /* _HEAP_H_ */

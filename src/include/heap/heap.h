
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
    // 堆分配器
    ALLOCATOR *allocator;

protected:
public:
    HEAP(void);
    ~HEAP(void);
    // 初始化
    bool init(void);
    // 内存申请，单位为 Byte
    void *malloc(size_t _byte);
    // 内存释放
    void free(void *_p);
};

extern HEAP heap;

#endif /* _HEAP_H_ */

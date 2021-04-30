
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.h for Simple-XX/SimpleKernel.

#ifndef _HEAP_H_
#define _HEAP_H_

#include "stdint.h"
#include "stddef.h"
#include "slab.h"

// TODO: 添加对 VMM 的处理
class HEAP {
private:
    // 堆最大容量 4MB
    static constexpr const uint32_t HEAP_SIZE = 0x400000;
    // 管理算法的名称
    const char *name;
    static SLAB manage;

protected:
public:
    HEAP(void);
    ~HEAP(void);
    // 初始化
    int32_t init(void);
    //
    int32_t manage_init(void);
    // 内存申请，单位为 Byte
    void *malloc(size_t byte);
    // 内存释放
    void free(void *p);
    // 获取管理的内存大小，包括管理信息
    size_t get_total(void);
    // 获取块数量
    size_t get_block(void);
    // 获取空闲内存数量 单位为 byte
    size_t get_free(void);
};

__attribute__((__init_priority__(1300))) static HEAP heap;

#endif /* _HEAP_H_ */

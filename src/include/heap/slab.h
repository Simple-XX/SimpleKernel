
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.h for Simple-XX/SimpleKernel.

#ifndef _SLAB_H_
#define _SLAB_H_

#include "stdint.h"
#include "stddef.h"
#include "allocator.h"

// TODO: 面向对象重构
class slab_list_entry_t {
public:
    // 该内存块是否已经被申请
    size_t allocated;
    // 当前内存块的长度，不包括头长度
    size_t             len;
    slab_list_entry_t *next;
    slab_list_entry_t *prev;
};

class SLAB : ALLOCATOR {
private:
    static constexpr const uint32_t SLAB_UNUSED = 0x00;
    static constexpr const uint32_t SLAB_USED   = 0x01;
    // 最小空间
    static constexpr const uint32_t SLAB_MIN = 0x100;
    // 堆管理的内存大小 单位为 bytes
    size_t heap_total;
    // 堆中 block 的数量
    size_t block_count;
    // 堆节点链表
    slab_list_entry_t *slab_list;

    // 切分内存块，len 为调用者申请的大小，不包括头大小
    // 返回分割出来的管理头地址
    slab_list_entry_t *slab_split(slab_list_entry_t *entry, size_t len);
    // 合并内存块
    void slab_merge(slab_list_entry_t *list);
    // 寻找长度符合的项
    slab_list_entry_t *find_entry(size_t len);
    void               set_used(slab_list_entry_t *entry);
    void               set_unused(slab_list_entry_t *entry);

protected:
public:
    SLAB(const void *_addr, size_t _len);
    ~SLAB(void);
    int32_t init(const void *start, const size_t size);
    // 获取管理的内存大小，包括管理信息
    size_t get_total(void);
    // 获取空闲内存数量 单位为 byte
    size_t get_free(void);
    // 获取块数量
    size_t get_block(void);
    void * alloc(size_t _len);
    bool   alloc(void *_addr, size_t _len);
    void   free(void *_addr, size_t _len);
    size_t get_used_pages_count(void) const;
    size_t get_free_pages_count(void) const;
};

#endif /* _SLAB_H_ */

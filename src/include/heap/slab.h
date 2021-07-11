
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.h for Simple-XX/SimpleKernel.

#ifndef _SLAB_H_
#define _SLAB_H_

#include "stdint.h"
#include "stddef.h"
#include "allocator.h"

// SLAB 分配器
class SLAB : ALLOCATOR {
private:
    // 两级结构
    // 第二级保存相同大小的内存块的使用情况
    struct slab_t {
        // slab_t 结构的物理地址
        // TODO: 可以移动到最后，这样直接 `addr=&.addr` 就好了
        void *addr;
        // 长度，不包括自身大小 单位为 byte
        size_t len;
        // 链表指针
        slab_t *prev;
        slab_t *next;
        slab_t(void);
        ~slab_t(void);
        // 插入新节点
        void push_back(slab_t *_new_node);
        // 链表长度
        size_t size(void);
        bool   operator!=(const slab_t &_node);
    };

    // 第一级保存不同长度的内存块
    class slab_cache_t {
    private:
        // 在 _which 链表中查找长度符合的
        slab_t *find(const slab_t &_which, size_t _len);

    protected:
    public:
        // 长度，单位为 byte
        size_t len;
        // 全部使用的链表
        slab_t full;
        // 部分使用的链表
        slab_t part;
        // 未使用的链表
        slab_t free;
        // 查找长度符合的
        slab_t *find(size_t _len);
        // 移动到另一个链表
        void move(slab_t *_list);
        // 申请物理内存，加入 free 链表
        void alloc_pmm(size_t _len);
        // 释放物理内存 单位为页
        void free_pmm(size_t _len);
    };

    // 管理不同长度的内存
    // 根据下标计算
    enum LEN {
        LEN32 = 0,
        LEN64,
        LEN128,
        LEN256,
        LEN512,
        LEN1024,
        LEN2048,
        LEN4096,
        LEN8192,
        LEN16384,
        LEN32768,
        LEN65536 = 11,
    };
    // 32 << _idx
    // 最小为 32 bytes
    // 支持 32～(32<<(CACHAE_LEN-1)) bytes
    // slab_cache[0] 即为内存为 32 字节的 slab_t 结构链表
    static constexpr const size_t CACHAE_LEN = 12;
    slab_cache_t                  slab_cache[CACHAE_LEN];
    // 根据 _len 获取对应的 slab_cache 下标
    size_t get_idx(size_t _len);

protected:
public:
    SLAB(const void *_addr, size_t _len);
    ~SLAB(void);
    // 获取块数量
    size_t get_block(void);
    // _len: 以 byte 为单位
    void *alloc(size_t _len);
    // slab 不支持这个函数
    bool alloc(void *_addr, size_t _len);
    // slab 不使用第二个参数
    void   free(void *_addr, size_t _len);
    size_t get_used_count(void) const;
    size_t get_free_count(void) const;
};

#endif /* _SLAB_H_ */

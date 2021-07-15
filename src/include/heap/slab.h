
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.h for Simple-XX/SimpleKernel.

#ifndef _SLAB_H_
#define _SLAB_H_

#include "stdint.h"
#include "stddef.h"
#include "allocator.h"

// 针对常用的 size 分别建立含有三个链表对象(full, part, free)的数组
// 需要分配时在对应 size 数组中寻找一个合适的 chunk
// 查找方法 首先查 part，再查 free，再没有就申请新的空间，再在 free 中查
// full/part/free 是相对于 pmm 分配的一个或多个连续的页而言的

// SLAB 分配器
class SLAB : ALLOCATOR {
private:
    // 两级结构
    // 第二级保存相同大小的内存块的使用情况
    struct chunk_t {
        // 头节点标识
        static constexpr const uint64_t HEAD = 0xCDCD;
        // chunk_t 结构的物理地址
        void *addr;
        // 长度，不包括自身大小 单位为 byte
        size_t len;
        // 链表指针
        chunk_t *prev;
        chunk_t *next;
        // 在 _addr 处建立新的 chunk
        chunk_t(void);
        ~chunk_t(void);
        // 插入新节点
        void push_back(chunk_t *_new_node);
        // 链表长度
        size_t size(void);
        bool   operator==(const chunk_t &_node);
        bool   operator!=(const chunk_t &_node);
    };

    // 第一级保存不同长度的内存块
    class slab_cache_t {
    private:
        // 对节点进行移动
        void move(chunk_t *_node);
        // 申请物理内存，加入 free 链表，返回申请到的地址
        chunk_t *alloc_pmm(size_t _len);
        // 释放物理内存
        void free_pmm(void);
        // 在 _which 链表中查找长度符合的
        chunk_t *find(chunk_t *_which, size_t _len, bool _alloc);

    protected:
    public:
        // 长度，单位为 byte
        size_t len;
        // 这三个作为头节点使用，不会实际使用
        // 全部使用的链表
        chunk_t *full;
        // 部分使用的链表
        chunk_t *part;
        // 未使用的链表
        chunk_t *free;
        // 查找长度符合的
        chunk_t *find(size_t _len);
    };
    // chunk 大小
    static constexpr const size_t CHUNK_SIZE = sizeof(chunk_t);
    // 管理不同长度的内存
    // 根据下标计算
    enum LEN {
        LEN256 = 0,
        LEN512,
        LEN1024,
        LEN2048,
        LEN4096,
        LEN8192,
        LEN16384,
        LEN32768,
        LEN65536 = 8,
    };
    // 最小为 256 bytes
    static constexpr const size_t MIN   = 256;
    static constexpr const size_t SHIFT = 8;
    // 支持 256(256<<(CACHAE_LEN-1)) bytes
    // slab_cache[0] 即为内存为 256 字节的 chunk_t 结构链表
    static constexpr const size_t CACHAE_LEN = 9;
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

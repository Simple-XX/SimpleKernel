
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// firstfit.h for Simple-XX/SimpleKernel.

#ifndef _FIRTSTFIT_H_
#define _FIRTSTFIT_H_

#include "stdint.h"

// 块
typedef struct chunk_info {
    // 当前页的地址
    uint8_t *addr;
    // 拥有多少个连续的页
    uint32_t npages;
    // 物理页被引用的次数
    int32_t ref;
    // 当前页状态
    uint32_t flag;
} chunk_info_t;

// 一个仅在这里使用的简单循环链表
typedef struct list_entry {
    chunk_info_t       chunk_info;
    struct list_entry *next;
    struct list_entry *prev;
} ff_list_entry_t;

class FIRSTFIT {
private:
    static constexpr const uint32_t FF_USED   = 0x00;
    static constexpr const uint32_t FF_UNUSED = 0x01;
    // 物理内存起始地址
    uint8_t *addr_start;
    // 物理内存结束地址
    uint8_t *addr_end;
    // 物理内存页的总数量
    uint32_t phy_page_count;
    // 物理内存页的当前数量
    uint32_t phy_page_free_count;
    // 空闲链表的节点数量
    uint32_t node_num;
    // 空闲链表
    ff_list_entry_t *free_list;

protected:
public:
    FIRSTFIT(uint8_t *addr_start);
    ~FIRSTFIT(void);
    // 初始化
    int32_t init(uint32_t pages);
    // 按页分配
    void *alloc(size_t pages);
    // 按页释放
    void free(void *addr, size_t pages);
    // 空闲页数量
    size_t free_pages_count(void);
};

#endif /* _FIRTSTFIT_H_ */

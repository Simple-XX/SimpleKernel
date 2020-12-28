
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// firstfit.h for Simple-XX/SimpleKernel.

#ifndef _FIRTSTFIT_H_
#define _FIRTSTFIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "pmm.h"

// 块
typedef struct chunk_info {
    // 当前页的地址
    addr_t addr;
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
} list_entry_t;

typedef struct firstfit_manage {
    // 物理内存起始地址
    addr_t pmm_addr_start;
    // 物理内存结束地址
    addr_t pmm_addr_end;
    // 物理内存页的总数量
    uint32_t phy_page_count;
    // 物理内存页的当前数量
    uint32_t phy_page_free_count;
    // 空闲链表的节点数量
    uint32_t node_num;
    // 空闲链表
    list_entry_t *free_list;
} firstfit_manage_t;

// 用于管理物理地址
extern pmm_manage_t firstfit_manage;

// 分区管理，定义3个管理器，均使用first-fit算法
extern firstfit_manage_t ff_manage;

#ifdef __cplusplus
}
#endif

#endif /* _FIRTSTFIT_H_ */

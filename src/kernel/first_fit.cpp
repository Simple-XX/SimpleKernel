
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// first_fit.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "cstring.h"
#include "common.h"
#include "pmm.h"
#include "stdio.h"
#include "firstfit.h"
#include "list_tmp.hpp"

FIRSTFIT::FIRSTFIT(COMMON::physical_pages_t *_phy_pages)
    : phy_pages(_phy_pages) {
    page_count      = 0;
    page_free_count = 0;
    node_num        = 0;
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    return;
}

void FIRSTFIT::set_chunk(ff_list_entry_t &         chunk,
                         COMMON::physical_pages_t &mempage) {
    chunk.addr   = mempage.addr;
    chunk.npages = 1;
    chunk.ref    = mempage.ref;
    chunk.flag   = 0;
    return;
}

int32_t FIRSTFIT::init(uint32_t _pages) {
    page_count = _pages;
    // 初始化 list 信息
    // 初始化头节点
    list_init_head(list);
    set_chunk(list[0], phy_pages[0]);
    // 遍历所有物理页，如果是连续的则合并入同一个 chunk，否则新建一个 chunk
    // 迭代所有页，如果下一个的地 != 当前地址+PAGE_SIZE 则新建 chunk
    ff_list_entry_t *chunk = list;
    node_num               = 0;
    for (uint32_t i = 0; i < page_count; i++) {
        // 如果连续且 ref 相同
        if ((phy_pages[i].addr ==
             chunk->addr + chunk->npages * COMMON::PAGE_SIZE) &&
            (phy_pages[i].ref == chunk->ref)) {
            chunk->npages++;
        }
        // 没有连续或者 ref 不同
        else {
            // 新建 chunk
            set_chunk(list[i], phy_pages[i]);
            // 添加到链表
            list_add_before(list, &list[i]);
            chunk = &list[i];
            node_num++;
        }
    }
// #define DEBUG
#ifdef DEBUG
    // 输出所有内存段
    chunk = list;
    do {
        printf("addr: 0x%X, len: 0x%X, ref: 0x%X\n", chunk->addr,
               chunk->npages * COMMON::PAGE_SIZE, chunk->ref);
        chunk = list_next(chunk);
    } while (chunk != list);
    printf("node_num: 0x%X\n", node_num);
#undef DEBUG
#endif
    // 计算未使用的物理内存
    for (uint32_t i = 0; i < page_count; i++) {
        if (phy_pages[i].ref == 0) {
            page_free_count++;
        }
    }
    printf("First fit init.\n");
    return 0;
}

void *FIRSTFIT::alloc(size_t _pages) {
    void *           res_addr = nullptr;
    ff_list_entry_t *entry    = list;
    do {
        // 当前 chunk 空闲
        if (entry->ref == 0) {
            // 判断长度是否足够
            if (entry->npages >= _pages) {
                // 符合条件，对 chunk 进行分割
                // 如果剩余大小足够
                if (entry->npages - _pages > 1) {
                    // 添加为新的链表项
                    ff_list_entry_t *tmp =
                        (ff_list_entry_t *)(entry +
                                            node_num * sizeof(ff_list_entry_t));
                    tmp->addr   = entry->addr + _pages * COMMON::PAGE_SIZE;
                    tmp->npages = entry->npages - _pages;
                    tmp->ref    = 0;
                    tmp->flag   = 0;
                    list_add_after(entry, tmp);
                }
                // 不够的话直接分配
                entry->npages = _pages;
                entry->ref    = 1;
                entry->flag   = 0;
                page_free_count -= _pages;
                res_addr = entry->addr;
                break;
            }
        }
    } while ((entry = list_next(entry)) != list);

    return res_addr;
}

void FIRSTFIT::free(void *addr_start, size_t _pages) {
    ff_list_entry_t *entry = list;
    while (((entry = list_next(entry)) != list) &&
           (entry->addr != addr_start)) {
        ;
    }
    // 释放所有页
    if (entry->ref > 0) {
        entry->ref--;
    }
    // 如果于相邻链表有空闲的则合并
    // 后面
    if (entry->next != entry && entry->next->ref == 0) {
        ff_list_entry_t *next = entry->next;
        entry->npages += next->npages;
        next->npages = 0;
        list_del(next);
    }
    // 前面
    if (entry->prev != entry && entry->prev->ref == 0) {
        ff_list_entry_t *prev = entry->prev;
        prev->npages += entry->npages;
        entry->npages = 0;
        list_del(entry);
    }
    page_free_count += _pages;
    return;
}

size_t FIRSTFIT::free_pages_count(void) {
    return page_free_count;
}

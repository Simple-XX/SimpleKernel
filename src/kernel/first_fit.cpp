
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// first_fit.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "common.h"
#include "pmm.h"
#include "firstfit.h"
#include "list.hpp"

IO              FIRSTFIT::io;
ff_list_entry_t FIRSTFIT::list[COMMON::PMM_PAGE_MAX_SIZE];

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
    chunk.flag   = mempage.ref == 0 ? FF_UNUSED : FF_USED;
    return;
}

void FIRSTFIT::set_used(const void *start, const void *end, size_t ref) {
    size_t idx = 0;
    while (1) {
        if (phy_pages[idx].addr >= start && phy_pages[idx].addr < end) {
            phy_pages[idx].ref += ref;
        }
        if (phy_pages[idx].addr == end) {
            break;
        }
        idx++;
    }
    return;
}

int32_t FIRSTFIT::init(uint32_t _pages) {
    // TODO: 优化空间
    // 将内核使用的内存设为已使用
    set_used(COMMON::KERNEL_START_4K, COMMON::KERNEL_END_4K, 1);
    // 初始化 list 信息
    // 初始化头节点
    list_init_head(list);
    set_chunk(list[0], phy_pages[0]);
    // 遍历所有物理页，如果是连续的则合并入同一个 chunk，否则新建一个 chunk
    // 迭代所有页，如果下一个的地 != 当前地址+PAGE_SIZE 则新建 chunk
    ff_list_entry_t *chunk = list;
    uint32_t         num   = 1;
    for (uint32_t i = 0; i < _pages; i++) {
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
            num++;
        }
    }
// #define DEBUG
#ifdef DEBUG
    // 输出所有内存段
    chunk = list;
    do {
        io.printf("addr: 0x%X, len: 0x%X, ref: 0x%X\n", chunk->addr,
                  chunk->npages * COMMON::PAGE_SIZE, chunk->ref);
        chunk = list_next(chunk);
    } while (chunk != list);
#undef DEBUG
#endif
    // 计算未使用的物理内存
    uint32_t n = 0;
    for (uint32_t i = 0; i < _pages; i++) {
        if (phy_pages[i].ref == 0) {
            n++;
        }
    }
    // 填充管理结构
    page_count      = _pages;
    page_free_count = n;
    node_num        = num;
    io.printf("First fit init.\n");

    return 0;
}

void *FIRSTFIT::alloc(size_t _pages) {
    void *           res_addr = nullptr;
    ff_list_entry_t *entry    = list;
    do {
        // 当前 chunk 空闲
        if (entry->flag == FF_UNUSED) {
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
                    tmp->flag   = FF_UNUSED;
                    list_add_after(entry, tmp);
                }
                // 不够的话直接分配
                entry->npages = _pages;
                entry->ref    = 1;
                entry->flag   = FF_USED;
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
    if (--entry->ref == 0) {
        entry->flag = FF_UNUSED;
    }
    // 如果于相邻链表有空闲的则合并
    // 后面
    if (entry->next != entry && entry->next->flag == FF_UNUSED) {
        ff_list_entry_t *next = entry->next;
        entry->npages += next->npages;
        next->npages = 0;
        list_del(next);
    }
    // 前面
    if (entry->prev != entry && entry->prev->flag == FF_UNUSED) {
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

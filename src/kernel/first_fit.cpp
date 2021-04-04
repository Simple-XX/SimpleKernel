
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

static int32_t set_chunk(ff_list_entry_t &chunk, physical_pages_t &mempage,
                         size_t _idx) {
    chunk.addr   = mempage.addr[_idx];
    chunk.npages = 1;
    chunk.ref    = mempage.ref[_idx];
    chunk.flag   = mempage.ref[_idx] == 0 ? FF_UNUSED : FF_USED;
    return 0;
}

IO              FIRSTFIT::io;
ff_list_entry_t FIRSTFIT::list[PMM_PAGE_MAX_SIZE];

FIRSTFIT::FIRSTFIT(physical_pages_t &_phy_pages) : phy_pages(_phy_pages) {
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    return;
}

int32_t FIRSTFIT::init(uint32_t pages) {
    // 为每一个页初始化一个记录其信息的 chunk
    // 第一个 chunk 保存在内核结束处
    // TODO: 优化空间
    // 管理所有内存页需要的空间，供管理结构使用
    // 最坏情况下，每个物理页都是独立的，所以分配与页数量对应的空间
    uint32_t pmm_info_size = PMM_PAGE_MAX_SIZE * sizeof(ff_list_entry_t);
    bzero(list, pmm_info_size);
    // 将用于保存物理地址信息的内存标记为已使用
    // 计算内核使用内存的 phy_pages
    // 不能直接计算，因为可用内存之间可能会存在空洞
    uint32_t idx = 0;
    while (phy_pages.addr[idx] < KERNEL_START_4K) {
        idx++;
    }
#ifdef DEBUG
    io.printf("phy_pages[idx]: 0x%X\n", phy_pages[idx]);
#endif
    uint32_t idx_end = idx;
    while (phy_pages.addr[idx_end] < KERNEL_END_4K) {
        idx_end++;
    }
    while (idx < idx_end) {
        phy_pages.ref[idx++]++;
    }
    // 初始化 list 信息
    // 初始化头节点
    list_init_head(list);
    set_chunk(list[0], phy_pages, 0);
    // 遍历所有物理页，如果是连续的则合并入同一个 chunk，否则新建一个 chunk
    // 迭代所有页，如果下一个的地 != 当前地址+PAGE_SIZE 则新建 chunk
    ff_list_entry_t *chunk = list;
    uint32_t         num   = 1;
    for (uint32_t i = 0; i < pages; i++) {
        // 如果连续且 ref 相同
        if ((phy_pages.addr[i] == chunk->addr + chunk->npages * PAGE_SIZE) &&
            (phy_pages.ref[i] == chunk->ref)) {
            chunk->npages++;
        }
        // 没有连续或者 ref 不同
        else {
            // 新建 chunk
            set_chunk(list[i], phy_pages, i);
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
        io.printf("addr: 0x%X, len: 0x%X, ref: 0x%X\n", chunk_info(chunk)->addr,
                  chunk_info(chunk)->npages * PAGE_SIZE,
                  chunk_info(chunk)->ref);
        chunk = list_next(chunk);
    } while (chunk != list);
#endif
    // 计算未使用的物理内存
    uint32_t n = 0;
    for (uint32_t i = 0; i < pages; i++) {
        if (phy_pages.ref[i] == 0) {
            n++;
        }
    }
    // 填充管理结构
    phy_page_count      = pages;
    phy_page_free_count = n;
    node_num            = num;
    io.printf("First fit init.\n");

    return 0;
}

void *FIRSTFIT::alloc(size_t pages) {
    void *           res_addr = nullptr;
    ff_list_entry_t *entry    = list;
    do {
        // 当前 chunk 空闲
        if (entry->flag == FF_UNUSED) {
            // 判断长度是否足够
            if (entry->npages >= pages) {
                // 符合条件，对 chunk 进行分割
                // 如果剩余大小足够
                if (entry->npages - pages > 1) {
                    // 添加为新的链表项
                    ff_list_entry_t *tmp =
                        (ff_list_entry_t *)(entry +
                                            node_num * sizeof(ff_list_entry_t));
                    tmp->addr   = entry->addr + pages * PAGE_SIZE;
                    tmp->npages = entry->npages - pages;
                    tmp->ref    = 0;
                    tmp->flag   = FF_UNUSED;
                    list_add_after(entry, tmp);
                }
                // 不够的话直接分配
                entry->npages = pages;
                entry->ref    = 1;
                entry->flag   = FF_USED;
                phy_page_free_count -= pages;
                res_addr = entry->addr;
                break;
            }
        }
    } while ((entry = list_next(entry)) != list);

    return res_addr;
}

void FIRSTFIT::free(void *addr_start, size_t pages) {
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
    phy_page_free_count += pages;
    return;
}

size_t FIRSTFIT::free_pages_count(void) {
    return phy_page_free_count;
}

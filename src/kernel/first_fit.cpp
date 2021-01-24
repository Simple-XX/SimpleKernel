
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// first_fit.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "io.h"
#include "string.h"
#include "pmm.h"
#include "firstfit.h"
#include "list.hpp"

// 返回 chunk_info
static chunk_info_t *chunk_info(ff_list_entry_t *list) {
    return &(list->chunk_info);
}

static int32_t set_chunk(ff_list_entry_t *chunk, physical_page_t *mempage) {
    chunk_info(chunk)->addr   = mempage->addr;
    chunk_info(chunk)->npages = 1;
    chunk_info(chunk)->ref    = mempage->ref;
    chunk_info(chunk)->flag   = mempage->ref == 0 ? FF_UNUSED : FF_USED;
    return 0;
}

FIRSTFIT::FIRSTFIT(uint8_t *addr_start) {
    this->addr_start = addr_start;
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    return;
}

int32_t FIRSTFIT::init(uint32_t pages) {
    // 为每一个页初始化一个记录其信息的 chunk
    // 第一个 chunk 保存在内核结束处
    // TODO: 优化空间
    ff_list_entry_t *pmm_info = (ff_list_entry_t *)addr_start;
    // 管理所有内存页需要的空间，供管理结构使用
    // 最坏情况下，每个物理页都是独立的，所以分配与页数量对应的空间
    uint32_t pmm_info_size = pages * sizeof(ff_list_entry_t);
    bzero(pmm_info, pmm_info_size);
    // 将用于保存物理地址信息的内存标记为已使用
    // 计算内核结束处对应的 phy_pages 下标与 pmm_info 使用内存结束处的下标
    // 不能直接计算，因为可用内存之间可能会存在空洞
    uint32_t idx = 0;
    while (phy_pages[idx].addr < addr_start) {
        idx++;
    }
#ifdef DEBUG
    io.printf("phy_pages[idx]: 0x%X\n", phy_pages[idx]);
#endif
    uint32_t idx_end = idx;
    while (phy_pages[idx_end].addr < addr_start + ALIGN4K(pmm_info_size)) {
        idx_end++;
    }
    while (idx < idx_end) {
        phy_pages[idx++].ref++;
    }
    // 初始化 free_list 信息
    // 初始化头节点
    list_init_head(pmm_info);
    set_chunk(pmm_info, &phy_pages[0]);
    // 遍历所有物理页，如果是连续的则合并入同一个 chunk，否则新建一个 chunk
    // 迭代所有页，如果下一个的地 != 当前地址+PMM_PAGE_SIZE 则新建 chunk
    ff_list_entry_t *chunk = pmm_info;
    uint32_t         num   = 1;
    for (uint32_t i = 0; i < pages; i++) {
        // 如果连续且 ref 相同
        if ((phy_pages[i].addr ==
             chunk_info(chunk)->addr +
                 chunk_info(chunk)->npages * PMM_PAGE_SIZE) &&
            (phy_pages[i].ref == chunk_info(chunk)->ref)) {
            chunk_info(chunk)->npages++;
        }
        // 没有连续或者 ref 不同
        else {
            // 新建 chunk
            ff_list_entry_t *tmp =
                (ff_list_entry_t *)((uint8_t *)pmm_info +
                                    i * sizeof(ff_list_entry_t));
            set_chunk(tmp, &phy_pages[i]);
            // 添加到链表
            list_add_before(pmm_info, tmp);
            chunk = tmp;
            num++;
        }
    }
// #define DEBUG
#ifdef DEBUG
    // 输出所有内存段
    chunk = pmm_info;
    do {
        io.printf("addr: 0x%X, len: 0x%X, ref: 0x%X\n", chunk_info(chunk)->addr,
                  chunk_info(chunk)->npages * PMM_PAGE_SIZE,
                  chunk_info(chunk)->ref);
        chunk = list_next(chunk);
    } while (chunk != pmm_info);
#endif
    // 计算未使用的物理内存
    uint32_t n = 0;
    for (uint32_t i = 0; i < pages; i++) {
        if (phy_pages[i].ref == 0) {
            n++;
        }
    }
    // 填充管理结构
    phy_page_count      = pages;
    phy_page_free_count = n;
    node_num            = num;
    free_list           = pmm_info;
    io.printf("First Fit management data end: 0x%X.\n",
              addr_start + ALIGN4K(pmm_info_size));
    io.printf("First fit init.\n");

    return 0;
}

void *FIRSTFIT::alloc(size_t pages) {
    void *           res_addr = NULL;
    ff_list_entry_t *entry    = free_list;
    do {
        // 当前 chunk 空闲
        if (chunk_info(entry)->flag == FF_UNUSED) {
            // 判断长度是否足够
            if (chunk_info(entry)->npages >= pages) {
                // 符合条件，对 chunk 进行分割
                // 如果剩余大小足够
                if (chunk_info(entry)->npages - pages > 1) {
                    // 添加为新的链表项
                    ff_list_entry_t *tmp =
                        (ff_list_entry_t *)(entry +
                                            node_num * sizeof(ff_list_entry_t));
                    chunk_info(tmp)->addr =
                        entry->chunk_info.addr + pages * PMM_PAGE_SIZE;
                    chunk_info(tmp)->npages = entry->chunk_info.npages - pages;
                    chunk_info(tmp)->ref    = 0;
                    chunk_info(tmp)->flag   = FF_UNUSED;
                    list_add_after(entry, tmp);
                }
                // 不够的话直接分配
                chunk_info(entry)->npages = pages;
                chunk_info(entry)->ref    = 1;
                chunk_info(entry)->flag   = FF_USED;
                phy_page_free_count -= pages;
                res_addr = chunk_info(entry)->addr;
                break;
            }
        }
    } while ((entry = list_next(entry)) != free_list);

    return res_addr;
}

void FIRSTFIT::free(void *addr_start, size_t pages) {
    ff_list_entry_t *entry = free_list;
    while (((entry = list_next(entry)) != free_list) &&
           (chunk_info(entry)->addr != addr_start)) {
        ;
    }
    // 释放所有页
    if (--chunk_info(entry)->ref == 0) {
        chunk_info(entry)->flag = FF_UNUSED;
    }
    // 如果于相邻链表有空闲的则合并
    // 后面
    if (entry->next != entry && chunk_info(entry->next)->flag == FF_UNUSED) {
        ff_list_entry_t *next = entry->next;
        chunk_info(entry)->npages += chunk_info(next)->npages;
        chunk_info(next)->npages = 0;
        list_del(next);
    }
    // 前面
    if (entry->prev != entry && chunk_info(entry->prev)->flag == FF_UNUSED) {
        ff_list_entry_t *prev = entry->prev;
        chunk_info(prev)->npages += chunk_info(entry)->npages;
        chunk_info(entry)->npages = 0;
        list_del(entry);
    }
    phy_page_free_count += pages;
    return;
}

size_t FIRSTFIT::free_pages_count(void) {
    return phy_page_free_count;
}

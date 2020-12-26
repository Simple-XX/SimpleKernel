
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// first_fit.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "firstfit.h"

#define FF_USED (0x00UL)
#define FF_UNUSED (0x01UL)

// 初始化
static void init(uint32_t pages);
// 按页分配
static ptr_t alloc(uint32_t pages);
// 按页释放
static void free(ptr_t addr_start, uint32_t pages);
// 空闲数量
static uint32_t free_pages_count(void);

pmm_manage_t firstfit_manage = {"Fitst Fit", &init, &alloc, &free,
                                &free_pages_count};

firstfit_manage_t ff_manage;

static inline void list_init_head(list_entry_t *list);

// 在中间添加元素
static inline void list_add_middle(list_entry_t *prev, list_entry_t *next,
                                   list_entry_t *new);

// 在 prev 后添加项
static inline void list_add_after(list_entry_t *prev, list_entry_t *new);

// 在 next 前添加项
static inline void list_add_before(list_entry_t *next, list_entry_t *new);

// 删除元素
static inline void list_del(list_entry_t *list);

// 返回前面的元素
static inline list_entry_t *list_prev(list_entry_t *list);

// 返回后面的的元素
static inline list_entry_t *list_next(list_entry_t *list);

// 返回 chunk_info
static inline chunk_info_t *list_chunk_info(list_entry_t *list);

// 初始化
void list_init_head(list_entry_t *list) {
    list->next = list;
    list->prev = list;
    return;
}

// 在中间添加元素
void list_add_middle(list_entry_t *prev, list_entry_t *next,
                     list_entry_t *new) {
    next->prev = new;
    new->next  = next;
    new->prev  = prev;
    prev->next = new;
    return;
}

// 在 prev 后添加项
void list_add_after(list_entry_t *prev, list_entry_t *new) {
    list_add_middle(prev, prev->next, new);
    return;
}

// 在 next 前添加项
void list_add_before(list_entry_t *next, list_entry_t *new) {
    list_add_middle(next->prev, next, new);
    return;
}

// 删除元素
void list_del(list_entry_t *list) {
    list->next->prev = list->prev;
    list->prev->next = list->next;
    return;
}

// 返回前面的元素
list_entry_t *list_prev(list_entry_t *list) {
    return list->prev;
}

// 返回后面的的元素
list_entry_t *list_next(list_entry_t *list) {
    return list->next;
}

// 返回 chunk_info
chunk_info_t *list_chunk_info(list_entry_t *list) {
    return &(list->chunk_info);
}

static int32_t set_chunk(list_entry_t *chunk, physical_page_t *mempage) {
    list_chunk_info(chunk)->addr   = mempage->addr;
    list_chunk_info(chunk)->npages = 1;
    list_chunk_info(chunk)->ref    = mempage->ref;
    list_chunk_info(chunk)->flag   = mempage->ref == 0 ? FF_UNUSED : FF_USED;
    return 0;
}

void init(uint32_t pages) {
    // 为每一个页初始化一个记录其信息的 chunk
    // 第一个 chunk 保存在内核结束处
    // TODO: 优化空间
    list_entry_t *pmm_info = (list_entry_t *)kernel_end_align4k;
    // 管理所有内存页需要的空间，供管理结构使用
    // 最坏情况下，每个物理页都是独立的，所以分配与页数量对应的空间
    uint32_t pmm_info_size = pages * sizeof(list_entry_t);
    bzero(pmm_info, pmm_info_size);
    // 将用于保存物理地址信息的内存标记为已使用
    // 计算内核结束处对应的 mem_page 下标与 pmm_info 使用内存结束处的下标
    // 不能直接计算，因为可用内存之间可能会存在空洞
    uint32_t idx = 0;
    while (mem_page[idx].addr < kernel_end_align4k) {
        idx++;
    }
#ifdef DEBUG
    printk_debug("mem_page[idx]: 0x%X\n", mem_page[idx]);
#endif
    uint32_t idx_end = idx;
    while (mem_page[idx_end].addr <
           kernel_end_align4k + ALIGN4K(pmm_info_size)) {
        idx_end++;
    }
    while (idx < idx_end) {
        mem_page[idx++].ref++;
    }
    // 初始化 free_list 信息
    // 初始化头节点
    list_init_head(pmm_info);
    set_chunk(pmm_info, &mem_page[0]);
    // 遍历所有物理页，如果是连续的则合并入同一个 chunk，否则新建一个 chunk
    // 迭代所有页，如果下一个的地 != 当前地址+PMM_PAGE_SIZE 则新建 chunk
    list_entry_t *chunk = pmm_info;
    uint32_t      num   = 1;
    for (uint32_t i = 1; i < pages; i++) {
        // 如果连续且 ref 相同
        if ((mem_page[i].addr ==
             list_chunk_info(chunk)->addr +
                 list_chunk_info(chunk)->npages * PMM_PAGE_SIZE) &&
            (mem_page[i].ref == list_chunk_info(chunk)->ref)) {
            list_chunk_info(chunk)->npages++;
        }
        // 没有连续或者 ref 不同
        else {
            // 新建 chunk
            list_entry_t *tmp =
                (list_entry_t *)((ptr_t)pmm_info + i * sizeof(list_entry_t));
            set_chunk(tmp, &mem_page[i]);
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
        printk_debug("addr: 0x%X, len: 0x%X, ref: 0x%X\n",
                     list_chunk_info(chunk)->addr,
                     list_chunk_info(chunk)->npages * PMM_PAGE_SIZE,
                     list_chunk_info(chunk)->ref);
        chunk = list_next(chunk);
    } while (chunk != pmm_info);
#endif
    // 计算未使用的物理内存
    uint32_t n = 0;
    for (uint32_t i = 0; i < pages; i++) {
        if (mem_page[i].ref == 0) {
            n++;
        }
    }
    // 填充管理结构
    ff_manage.phy_page_count      = pages;
    ff_manage.phy_page_free_count = n;
    ff_manage.node_num            = num;
    ff_manage.free_list           = pmm_info;
    printk_info("First Fit management data end: 0x%X.\n",
                kernel_end_align4k + ALIGN4K(pmm_info_size));
    printk_info("First fit init.\n");

    return;
}

ptr_t alloc(uint32_t pages) {
    ptr_t         res_addr = 0x00;
    list_entry_t *entry    = ff_manage.free_list;
    // do {
    //     // 当前 chunk 空闲
    //     if (list_chunk_info(entry)->flag == FF_UNUSED) {
    //         // 判断长度是否足够
    //         if (list_chunk_info(entry)->npages >= pages) {
    //             // 符合条件，对 chunk 进行分割
    //         }
    //     }
    // } while ((entry = list_next(entry)) != ff_manage.free_list);
    while (1) {
        // 查找符合长度且未使用的内存
        if ((list_chunk_info(entry)->npages >= pages) &&
            (list_chunk_info(entry)->flag == FF_UNUSED)) {
            // 如果剩余大小足够
            if (list_chunk_info(entry)->npages - pages > 1) {
                // 添加为新的链表项
                list_entry_t *tmp =
                    (list_entry_t *)(entry +
                                     ff_manage.node_num * sizeof(list_entry_t));
                list_chunk_info(tmp)->addr =
                    entry->chunk_info.addr + pages * PMM_PAGE_SIZE;
                list_chunk_info(tmp)->npages = entry->chunk_info.npages - pages;
                list_chunk_info(tmp)->ref    = 0;
                list_chunk_info(tmp)->flag   = FF_UNUSED;
                list_add_after(entry, tmp);
            }
            // 不够的话直接分配
            list_chunk_info(entry)->npages = pages;
            list_chunk_info(entry)->ref    = 1;
            list_chunk_info(entry)->flag   = FF_USED;
            ff_manage.phy_page_free_count -= pages;
            res_addr = list_chunk_info(entry)->addr;
            break;
        }
        // 没找到的话就查找下一个
        else if (list_next(entry) != ff_manage.free_list) {
            entry = list_next(entry);
        }
        // 执行到这里还没找见的话，物理内存已经全部查找过了，说明物理内存不够
        else {
            printk_err("No enough phy mem.\n");
            res_addr = -1;
            break;
        }
    }
    return res_addr;
}

void free(ptr_t addr_start, uint32_t pages) {
    list_entry_t *entry = ff_manage.free_list;
    while (((entry = list_next(entry)) != ff_manage.free_list) &&
           (list_chunk_info(entry)->addr != addr_start)) {
        ;
    }
    // 释放所有页
    if (--list_chunk_info(entry)->ref == 0) {
        list_chunk_info(entry)->flag = FF_UNUSED;
    }
    // 如果于相邻链表有空闲的则合并
    // 后面
    if (entry->next != entry &&
        list_chunk_info(entry->next)->flag == FF_UNUSED) {
        list_entry_t *next = entry->next;
        list_chunk_info(entry)->npages += list_chunk_info(next)->npages;
        list_chunk_info(next)->npages = 0;
        list_del(next);
    }
    // 前面
    if (entry->prev != entry &&
        list_chunk_info(entry->prev)->flag == FF_UNUSED) {
        list_entry_t *prev = entry->prev;
        list_chunk_info(prev)->npages += list_chunk_info(entry)->npages;
        list_chunk_info(entry)->npages = 0;
        list_del(entry);
    }
    ff_manage.phy_page_free_count += pages;
    return;
}

uint32_t free_pages_count(void) {
    return ff_manage.phy_page_free_count;
}

#ifdef __cplusplus
}
#endif


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
static void init(int pages);
// 分配
static ptr_t alloc(uint32_t bytes);
// 释放
static void free(ptr_t addr_start, uint32_t bytes);
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

void init(int pages) {
    // 为每一个页初始化一个记录其信息的 chunk
    // 第一个 chunk 保存在内核结束处，按 4k 对齐
    list_entry_t *pmm_info =
        (list_entry_t *)(((ptr_t)(&kernel_end) & PMM_PAGE_MASK));
    // 最差情况，一块只有一个页，所以预先留好空间存储这些块信息
    // 管理所有内存页需要的空间，供管理结构使用
    // uint32_t pmm_info_size = mem_zone[DMA].all_pages * sizeof(list_entry_t);
    // bzero(pmm_info, pmm_info_size);
    //管理信息也需占用页表
    // for (unsigned int i = DMA_START_ADDR;
    //      i < DMA_START_ADDR + dma_pmm_info_size; i += PMM_PAGE_SIZE) {
    //     mem_page[i / (unsigned int)PMM_PAGE_SIZE].ref = 1;
    // }
    // for (unsigned int i = NORMAL_START_ADDR;
    //      i < NORMAL_START_ADDR + normal_pmm_info_size; i += PMM_PAGE_SIZE) {
    //     mem_page[i / (unsigned int)PMM_PAGE_SIZE].ref = 1;
    // }
    // for (unsigned int i = HIGHMEM_START_ADDR;
    //      i < HIGHMEM_START_ADDR + highmem_pmm_info_size; i += PMM_PAGE_SIZE)
    //      {
    //     mem_page[i / (unsigned int)PMM_PAGE_SIZE].ref = 1;
    // }

    // for (int i = 0; i < pages; i++) {
    //     ;
    // }
#ifdef DEBUG
#endif /* DEBUG */
    printk_info("First fit init.\n");
    return;
}

// 根据线性地址判断属于那个管理区，然后使用对应的物理分区管理器进行分配。
ptr_t alloc(uint32_t bytes) {
    // 计算需要的页数
    size_t pages = bytes / PMM_PAGE_SIZE;
    // 不足一页的 + 1
    if (bytes % PMM_PAGE_SIZE != 0) {
        pages += 1;
    }
    ptr_t         res_addr = 0;
    list_entry_t *entry    = ff_manage.free_list;
    // while (entry >= 0) {
    while (1) {
        // printk_info("successful-2!\n");
        // 查找符合长度且未使用的内存
        // printk_info("addr:%08X\n",list_chunk_info(entry));
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
            ff_manage.phy_page_now_count -= pages;
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

void free(ptr_t addr_start, uint32_t bytes) {
    // 计算需要的页数
    size_t pages = bytes / PMM_PAGE_SIZE;
    // 不足一页的+1
    if (bytes % PMM_PAGE_SIZE != 0) {
        pages++;
    }
    list_entry_t *entry = ff_manage.free_list;
    while (((entry = list_next(entry)) != ff_manage.free_list) &&
           (list_chunk_info(entry)->addr != addr_start)) {
        ;
    }

    // 释放所有页
    list_chunk_info(entry)->ref  = 0;
    list_chunk_info(entry)->flag = FF_UNUSED;

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
    ff_manage.phy_page_now_count += pages;
    return;
}

uint32_t free_pages_count(void) {
    return ff_manage.phy_page_now_count;
}

#ifdef __cplusplus
}
#endif

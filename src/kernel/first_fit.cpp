
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// first_fit.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "common.h"
#include "pmm.h"
#include "stdio.h"
#include "firstfit.h"
#include "list.hpp"

FIRSTFIT::FIRSTFIT(const void *_addr, size_t _len)
    : ALLOCATOR(_addr, _len), list(const_cast<void *>(COMMON::PMM_INFO_START),
                                   const_cast<void *>(COMMON::PMM_INFO_END)) {
    name           = (char *)"PMM(First Fit) allocator";
    list[0].addr   = const_cast<void *>(_addr);
    list[0].npages = _len;
    list[0].used   = false;
    list.size      = 1;
    printf("%s init.\n", name);
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    return;
}

void *FIRSTFIT::alloc(size_t _len) {
    void *res_addr = nullptr;
    // 遍历链表
    for (size_t i = 0; i < list.size; i++) {
// #define DEBUG
#ifdef DEBUG
        printf("list[0x%X].addr: 0x%p\n", i, list[i].addr);
        printf("list[0x%X].npages: 0x%p\n", i, list[i].npages);
        printf("list[0x%X].used: 0x%p\n", i, list[i].used);
        printf("list.size: 0x%X\n", list.size);
#undef DEBUG
#endif
        // 寻找长度符合的表项，并且引用数为 0，即未使用
        if (list[i].npages >= _len && list[i].used == false) {
            // 保存要返回的地址
            res_addr = list[i].addr;
            // 将剩余页新建链表项
            if (list[i].npages > _len) {
                ff_entry_t new_data;
                new_data.addr   = (void *)((uint8_t *)list[i].addr +
                                         _len * COMMON::PAGE_SIZE);
                new_data.npages = list[i].npages - _len;
                new_data.used   = false;
                list.add_after(list[i], new_data);
            }
            // 要返回表项的数据进行更新
            list[i].npages = _len;
            list[i].used   = true;
            free_pages_count -= _len;
            used_pages_count += _len;
            // 返回
            return res_addr;
        }
    }
    // 执行到这里说明没有足够空间了，返回空指针
    err("NO ENOUGH MEMOTY.\n");
    return nullptr;
}

bool FIRSTFIT::alloc(void *_addr, size_t _len) {
    return 0;
}

void FIRSTFIT::free(void *_addr, size_t _len) {
    // 释放需要考虑前后的合并
    // 首先定位到对应的 node
    return;
}

size_t FIRSTFIT::get_used_pages_count(void) const {
    return used_pages_count;
}

size_t FIRSTFIT::get_free_pages_count(void) const {
    return free_pages_count;
}

// void FIRSTFIT::free(void *addr_start, size_t _pages) {
//     ff_list_entry_t *entry = list;
//     while (((entry = list_next(entry)) != list) &&
//            (entry->addr != addr_start)) {
//         ;
//     }
//     // 释放所有页
//     if (entry->ref > 0) {
//         entry->ref--;
//     }
//     // 如果于相邻链表有空闲的则合并
//     // 后面
//     if (entry->next != entry && entry->next->ref == 0) {
//         ff_list_entry_t *next = entry->next;
//         entry->npages += next->npages;
//         next->npages = 0;
//         list_del(next);
//     }
//     // 前面
//     if (entry->prev != entry && entry->prev->ref == 0) {
//         ff_list_entry_t *prev = entry->prev;
//         prev->npages += entry->npages;
//         entry->npages = 0;
//         list_del(entry);
//     }
//     page_free_count += _pages;
//     return;
// }

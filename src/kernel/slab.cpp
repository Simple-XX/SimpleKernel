
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "list.hpp"
#include "common.h"
#include "pmm.h"
#include "slab.h"

SLAB::SLAB(const void *_addr, size_t _len) : ALLOCATOR(_addr, _len) {
    name = (char *)"HEAP(SLAB) allocator";
    // 设置第一块内存的信息
    // 首先给链表中添加一个大小为 1 页的块
    slab_list = (slab_list_entry_t *)PMM::alloc_page();
    bzero(slab_list, COMMON::PAGE_SIZE);
    list_init_head(slab_list);
    // 设置第一块内存的相关信息
    slab_list->allocated = SLAB_UNUSED;
    slab_list->len       = COMMON::PAGE_SIZE - sizeof(slab_list_entry_t);
    block_count          = 1;
    printf("%s init.\n", name);
    return;
}

SLAB::~SLAB(void) {
    return;
}

slab_list_entry_t *SLAB::slab_split(slab_list_entry_t *entry, size_t len) {
    // 如果剩余内存大于内存头的长度+设定的最小长度
    if (entry->len - len > sizeof(slab_list_entry_t) + SLAB_MIN) {
        // 添加新的链表项，位于旧表项开始地址+旧表项长度
        slab_list_entry_t *new_entry =
            (slab_list_entry_t *)((uint8_t *)entry + sizeof(slab_list_entry_t) +
                                  len);

        bzero((void *)new_entry, entry->len - len);
        list_init_head(new_entry);
        // 新表项的长度为：list->len（总大小）- 头大小 - 要求分割的大小
        new_entry->len = entry->len - len - sizeof(slab_list_entry_t);
        set_unused(new_entry);
        list_add_after(entry, new_entry);
        // 重新设置旧链表信息
        entry->len = len;
        block_count++;
        return new_entry;
    }
    return (slab_list_entry_t *)nullptr;
}

void SLAB::slab_merge(slab_list_entry_t *list) {
    slab_list_entry_t *entry = list;
    slab_list_entry_t *prev  = entry->prev;
    slab_list_entry_t *next  = entry->next;
    slab_list_entry_t *tmp   = nullptr;
    // 合并后面的
    if (next != list && next->allocated == SLAB_UNUSED) {
        // 是否连续
        if (reinterpret_cast<ptrdiff_t>(entry) + entry->len +
                sizeof(slab_list_entry_t) ==
            (unsigned)reinterpret_cast<ptrdiff_t>(next)) {
            entry->len += next->len + sizeof(slab_list_entry_t);
            list_del(next);
            block_count--;
            tmp = next;
        }
    }
    // 合并前面的
    if (prev != list && prev->allocated == SLAB_UNUSED) {
        // 是否连续
        if (reinterpret_cast<ptrdiff_t>(prev) + prev->len +
                sizeof(slab_list_entry_t) ==
            (unsigned)reinterpret_cast<ptrdiff_t>(entry)) {
            prev->len += (entry->len + sizeof(slab_list_entry_t));
            list_del(entry);
            block_count--;
            tmp = prev;
        }
    }
    if ((tmp != nullptr) &&
        (tmp->len > sizeof(slab_list_entry_t) + COMMON::PAGE_SIZE * 2)) {
        // 要回收页的地址
        void *addr = (void *)COMMON::ALIGN(tmp, 4 * COMMON::KB);
        // 需要回收几页
        size_t count = tmp->len / COMMON::PAGE_SIZE;
        // 回收
        PMM::free_pages(addr, count);
        // heap_total -= count * COMMON::PAGE_SIZE;
        // 如果 tmp 从页首开始，则删除 block，tmp 为 slab_list 时除外
        if ((reinterpret_cast<ptrdiff_t>(tmp) % COMMON::PAGE_SIZE == 0) &&
            (tmp != slab_list)) {
            // 删除 block
            list_del(tmp);
            block_count--;
        }
        else {
            // 重新设置 len
            tmp->len -= count * COMMON::PAGE_SIZE;
        }
    }
    return;
}

slab_list_entry_t *SLAB::find_entry(size_t len) {
    slab_list_entry_t *entry = slab_list;
    do {
        // 查找符合长度且未使用，符合对齐要求的内存
        if ((entry->len >= len) && (entry->allocated == SLAB_UNUSED)) {
            // 进行分割，这个函数会同时设置 entry 的信息
            slab_split(entry, len);
            return entry;
        }
    } while ((entry = list_next(entry)) != slab_list);
    return (slab_list_entry_t *)nullptr;
}

// 将 entry 设置为已使用
void SLAB::set_used(slab_list_entry_t *entry) {
    entry->allocated = SLAB_USED;
    return;
}

// 将 entry 设置为未使用
void SLAB::set_unused(slab_list_entry_t *entry) {
    entry->allocated = SLAB_UNUSED;
    return;
}

void *SLAB::alloc(size_t _len) {
    // 所有申请的内存长度(限制最小大小)加上管理头的长度
    size_t len = (_len > SLAB_MIN) ? _len : SLAB_MIN;
    // len 对齐
    len = (len + SLAB_MIN - 1) & (0xFFFFFFFFFFFFFFFF - SLAB_MIN + 1);
    slab_list_entry_t *entry = find_entry(len);
    if (entry != nullptr) {
        set_used(entry);
        return (void *)((uint8_t *)entry + sizeof(slab_list_entry_t));
    }
    entry = list_prev(slab_list);
    // 如果执行到这里，说明没有可用空间了，那么申请新的内存页
    len += sizeof(slab_list_entry_t);
    size_t             pages     = (len % COMMON::PAGE_SIZE == 0)
                                       ? (len / COMMON::PAGE_SIZE)
                                       : ((len / COMMON::PAGE_SIZE) + 1);
    slab_list_entry_t *new_entry = (slab_list_entry_t *)PMM::alloc_pages(pages);
    if (new_entry == nullptr) {
        printf("Error at slab.c void *alloc(): no enough physical memory\n");
        return nullptr;
    }
    list_init_head(new_entry);
    // 新表项的可用长度为减去头的大小
    new_entry->len = pages * COMMON::PAGE_SIZE - sizeof(slab_list_entry_t);
    list_add_after(entry, new_entry);
    block_count++;
    // 进行分割
    slab_split(new_entry, len);
    set_used(new_entry);
    return (void *)((uint8_t *)new_entry + sizeof(slab_list_entry_t));
}

bool SLAB::alloc(void *, size_t) {
    return true;
}

void SLAB::free(void *_addr, size_t) {
    // 获取实际开始地址
    slab_list_entry_t *entry =
        (slab_list_entry_t *)((uint8_t *)_addr - sizeof(slab_list_entry_t));
    if (entry->allocated != SLAB_USED) {
        printf("Error at slab.c void free(void *)\n");
        return;
    }
    entry->allocated = SLAB_UNUSED;
    slab_merge(entry);
    return;
}

size_t SLAB::get_block(void) {
    return block_count;
}

size_t SLAB::get_used_count(void) const {
    return 0;
}

size_t SLAB::get_free_count(void) const {
    size_t             f     = 0;
    slab_list_entry_t *entry = slab_list;
    do {
        if (entry->allocated == SLAB_UNUSED) {
            f += entry->len;
        }
    } while ((entry = list_next(entry)) != slab_list);
    return f;
}

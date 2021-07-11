
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "pmm.h"
#include "slab.h"

SLAB::slab_t::slab_t(void) {
    addr = nullptr;
    len  = 0;
    prev = this;
    next = this;
    return;
}

SLAB::slab_t::~slab_t(void) {
    return;
}

bool SLAB::slab_t::operator!=(const slab_t &_node) {
    return addr == _node.addr && len == _node.len && prev == _node.prev &&
           next == _node.prev;
}

// 由于是循环队列，相当于在头节点前面插入
void SLAB::slab_t::push_back(slab_t *_new_node) {
    _new_node->next = this;
    _new_node->prev = prev;
    prev->next      = _new_node;
    prev            = _new_node;
    return;
}

SLAB::slab_t *SLAB::slab_cache_t::find(const slab_t &_which, size_t _len) {
    printf("_which.addr: 0x%X, _which.len: 0x%X, _len: 0x%X\n", _which.addr,
           _which.len, _len);
    slab_t *res = nullptr;
    slab_t  tmp = _which;
    // 遍历 _which，寻找 len>= _len 的节点
    // BUG: 循环条件有问题
    do {
        if (tmp.len >= _len) {
            res = (slab_t *)tmp.addr;
            printf("res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
            break;
        }
        tmp = *tmp.next;
    } while (tmp != _which);
    printf("res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
    return res;
}

SLAB::slab_t *SLAB::slab_cache_t::find(size_t _len) {
    slab_t *node = nullptr;
    // 首先在 part 里找
    printf("_len: 0x%X\n", _len);
    node = find(part, _len);
    // 没找到的话在 free 里找
    if (node == nullptr) {
        node = find(free, _len);
    }
    // 也没有的话就申请新的物理页，并在 free 中重新查找
    if (node == nullptr) {
        alloc_pmm(_len);
        node = find(free, _len);
    }
    // 还没有的话说明出错了
    assert(node != nullptr);
    return node;
}

void SLAB::slab_cache_t::alloc_pmm(size_t _len) {
    // 计算页数
    size_t pages = _len / COMMON::PAGE_SIZE;
    if (_len % COMMON::PAGE_SIZE != 0) {
        pages += 1;
    }
    // 申请
    slab_t *new_node = (slab_t *)PMM::alloc_pages(pages);
    // 初始化
    // 自身的地址
    new_node->addr = (void *)new_node;
    // 长度需要减去 slab_t 的长度
    new_node->len = (pages * COMMON::PAGE_SIZE) - sizeof(slab_t);
    // 链表指针
    new_node->prev = new_node;
    new_node->next = new_node;
    // 插入 free 节点
    free.push_back(new_node);
    return;
}

size_t SLAB::get_idx(size_t _len) {
    size_t res = 0;
    // _len 向上取整
    _len += _len - 1;
    while (1) {
        // 每次循环右移一位
        _len = _len >> 1;
        res++;
        if (_len == 0) {
            // res 需要减去 2^5=32
            res -= 6;
            break;
        }
    }
    return res;
}

SLAB::SLAB(const void *_addr, size_t _len) : ALLOCATOR(_addr, _len) {
    name = (char *)"HEAP(SLAB) allocator";
    printf("%s init.\n", name);
    return;
}

SLAB::~SLAB(void) {
    return;
}

void *SLAB::alloc(size_t _len) {
    // 分配时，首先确定需要分配的大小
    auto bytes = _len + sizeof(slab_t);
    // 根据大小确定 slab_cache 索引
    auto idx = get_idx(bytes);
    printf("bytes: 0x%X, idx: 0x%X\n", bytes, idx);
    // 寻找合适的 slab 节点
    slab_t *node = slab_cache[idx].find(bytes);
    printf("node->addr: 0x%X, node->len: 0x%X\n", node->addr, node->len);
    // 计算地址
    void *res = (uint8_t *)node->addr + sizeof(slab_t);
    // 返回
    return res;
}

bool SLAB::alloc(void *, size_t) {
    return true;
}

void SLAB::free(void *, size_t) {
    return;
}

size_t SLAB::get_used_count(void) const {
    return 0;
}

size_t SLAB::get_free_count(void) const {
    return 0;
}

// slab_list_entry_t *SLAB::slab_split(slab_list_entry_t *entry, size_t len) {
//     // 如果剩余内存大于内存头的长度+设定的最小长度
//     if (entry->len - len > sizeof(slab_list_entry_t) + SLAB_MIN) {
//         // 添加新的链表项，位于旧表项开始地址+旧表项长度
//         slab_list_entry_t *new_entry =
//             (slab_list_entry_t *)((uint8_t *)entry +
//             sizeof(slab_list_entry_t) +
//                                   len);

//         bzero((void *)new_entry, entry->len - len);
//         list_init_head(new_entry);
//         // 新表项的长度为：list->len（总大小）- 头大小 - 要求分割的大小
//         new_entry->len = entry->len - len - sizeof(slab_list_entry_t);
//         set_unused(new_entry);
//         list_add_after(entry, new_entry);
//         // 重新设置旧链表信息
//         entry->len = len;
//         block_count++;
//         return new_entry;
//     }
//     return (slab_list_entry_t *)nullptr;
// }

// void SLAB::slab_merge(slab_list_entry_t *list) {
//     slab_list_entry_t *entry = list;
//     slab_list_entry_t *prev  = entry->prev;
//     slab_list_entry_t *next  = entry->next;
//     slab_list_entry_t *tmp   = nullptr;
//     // 合并后面的
//     if (next != list && next->allocated == SLAB_UNUSED) {
//         // 是否连续
//         if (reinterpret_cast<ptrdiff_t>(entry) + entry->len +
//                 sizeof(slab_list_entry_t) ==
//             (unsigned)reinterpret_cast<ptrdiff_t>(next)) {
//             entry->len += next->len + sizeof(slab_list_entry_t);
//             list_del(next);
//             block_count--;
//             tmp = next;
//         }
//     }
//     // 合并前面的
//     if (prev != list && prev->allocated == SLAB_UNUSED) {
//         // 是否连续
//         if (reinterpret_cast<ptrdiff_t>(prev) + prev->len +
//                 sizeof(slab_list_entry_t) ==
//             (unsigned)reinterpret_cast<ptrdiff_t>(entry)) {
//             prev->len += (entry->len + sizeof(slab_list_entry_t));
//             list_del(entry);
//             block_count--;
//             tmp = prev;
//         }
//     }
//     if ((tmp != nullptr) &&
//         (tmp->len > sizeof(slab_list_entry_t) + COMMON::PAGE_SIZE * 2)) {
//         // 要回收页的地址
//         void *addr = (void *)COMMON::ALIGN(tmp, 4 * COMMON::KB);
//         // 需要回收几页
//         size_t count = tmp->len / COMMON::PAGE_SIZE;
//         // 回收
//         PMM::free_pages(addr, count);
//         // heap_total -= count * COMMON::PAGE_SIZE;
//         // 如果 tmp 从页首开始，则删除 block，tmp 为 slab_list 时除外
//         if ((reinterpret_cast<ptrdiff_t>(tmp) % COMMON::PAGE_SIZE == 0) &&
//             (tmp != slab_list)) {
//             // 删除 block
//             list_del(tmp);
//             block_count--;
//         }
//         else {
//             // 重新设置 len
//             tmp->len -= count * COMMON::PAGE_SIZE;
//         }
//     }
//     return;
// }

// slab_list_entry_t *SLAB::find_entry(size_t len) {
//     slab_list_entry_t *entry = slab_list;
//     do {
//         // 查找符合长度且未使用，符合对齐要求的内存
//         if ((entry->len >= len) && (entry->allocated == SLAB_UNUSED)) {
//             // 进行分割，这个函数会同时设置 entry 的信息
//             slab_split(entry, len);
//             return entry;
//         }
//     } while ((entry = list_next(entry)) != slab_list);
//     return (slab_list_entry_t *)nullptr;
// }

// // 将 entry 设置为已使用
// void SLAB::set_used(slab_list_entry_t *entry) {
//     entry->allocated = SLAB_USED;
//     return;
// }

// // 将 entry 设置为未使用
// void SLAB::set_unused(slab_list_entry_t *entry) {
//     entry->allocated = SLAB_UNUSED;
//     return;
// }

// void *SLAB::alloc(size_t _len) {
//     // 所有申请的内存长度(限制最小大小)加上管理头的长度
//     size_t len = (_len > SLAB_MIN) ? _len : SLAB_MIN;
//     // len 对齐
//     len = (len + SLAB_MIN - 1) & (0xFFFFFFFFFFFFFFFF - SLAB_MIN + 1);
//     slab_list_entry_t *entry = find_entry(len);
//     if (entry != nullptr) {
//         set_used(entry);
//         return (void *)((uint8_t *)entry + sizeof(slab_list_entry_t));
//     }
//     entry = list_prev(slab_list);
//     // 如果执行到这里，说明没有可用空间了，那么申请新的内存页
//     len += sizeof(slab_list_entry_t);
//     size_t             pages     = (len % COMMON::PAGE_SIZE == 0)
//                                        ? (len / COMMON::PAGE_SIZE)
//                                        : ((len / COMMON::PAGE_SIZE) + 1);
//     slab_list_entry_t *new_entry = (slab_list_entry_t
//     *)PMM::alloc_pages(pages); if (new_entry == nullptr) {
//         printf("Error at slab.c void *alloc(): no enough physical memory\n");
//         return nullptr;
//     }
//     list_init_head(new_entry);
//     // 新表项的可用长度为减去头的大小
//     new_entry->len = pages * COMMON::PAGE_SIZE - sizeof(slab_list_entry_t);
//     list_add_after(entry, new_entry);
//     block_count++;
//     // 进行分割
//     slab_split(new_entry, len);
//     set_used(new_entry);
//     return (void *)((uint8_t *)new_entry + sizeof(slab_list_entry_t));
// }

// bool SLAB::alloc(void *, size_t) {
//     return true;
// }

// void SLAB::free(void *_addr, size_t) {
//     // 获取实际开始地址
//     slab_list_entry_t *entry =
//         (slab_list_entry_t *)((uint8_t *)_addr - sizeof(slab_list_entry_t));
//     if (entry->allocated != SLAB_USED) {
//         printf("Error at slab.c void free(void *)\n");
//         return;
//     }
//     entry->allocated = SLAB_UNUSED;
//     slab_merge(entry);
//     return;
// }

// size_t SLAB::get_block(void) {
//     return block_count;
// }

// size_t SLAB::get_used_count(void) const {
//     return 0;
// }

// size_t SLAB::get_free_count(void) const {
//     size_t             f     = 0;
//     slab_list_entry_t *entry = slab_list;
//     do {
//         if (entry->allocated == SLAB_UNUSED) {
//             f += entry->len;
//         }
//     } while ((entry = list_next(entry)) != slab_list);
//     return f;
// }

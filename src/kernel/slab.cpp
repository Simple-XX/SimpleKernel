
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "pmm.h"
#include "slab.h"

SLAB::chunk_t::chunk_t(void) {
    addr = nullptr;
    len  = 0;
    prev = this;
    next = this;
    return;
}

SLAB::chunk_t::~chunk_t(void) {
    return;
}

bool SLAB::chunk_t::operator==(const chunk_t &_node) {
    return addr == _node.addr && len == _node.len && prev == _node.prev &&
           next == _node.prev;
}

bool SLAB::chunk_t::operator!=(const chunk_t &_node) {
    return addr != _node.addr || len != _node.len || prev != _node.prev ||
           next != _node.prev;
}

// 由于是循环队列，相当于在头节点前面插入
void SLAB::chunk_t::push_back(chunk_t *_new_node) {
    _new_node->next = this;
    _new_node->prev = prev;
    prev->next      = _new_node;
    prev            = _new_node;
    return;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(chunk_t &_which, size_t _len) {
    chunk_t *res = nullptr;
    // 遍历 _which，寻找 len>= _len 的节点
    chunk_t tmp = _which;
    do {
        // 如果 len 大于 _len 即可
        // 执行到这里，_len <= len 成立
        // 只需要判断通过 alloc_pmm 申请的一页内存剩余空间足够即可
        if (tmp.len >= _len) {
            // 更新节点信息
            res       = (chunk_t *)tmp.addr;
            res->addr = tmp.addr;
            res->len  = _len;
            // 进行移动 例如 _which 为 free，那么 res 需要移动到 full
            move(res);
            // 这里的逻辑缺了一环
            // 在划分出 _len 后，需要新建一个节点保存剩余空间的信息
            // 建立新的节点
            // 判断剩余多少空间 如果剩余的空间大于 len 的话
            if (tmp.len - _len >= len) {
                // 新建一个节点
                // 计算新节点的地址
                chunk_t *new_chunk = (chunk_t *)((uint8_t *)tmp.addr + tmp.len +
                                                 sizeof(chunk_t));
                // 设置数据
                new_chunk->addr =
                    ((uint8_t *)tmp.addr + tmp.len + sizeof(chunk_t));
                new_chunk->len = tmp.len - _len;
                // 添加到链表
                _which.push_back(new_chunk);
                // 进行移动 例如 _which 为 free，那么 new_chunk 需要移动到 part
                move(new_chunk);
            }
            printf("res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
            break;
        }
        tmp = *tmp.next;
    } while (tmp != _which);
    return res;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(size_t _len) {
    printf("_len: 0x%X\n", _len);
    chunk_t *chunk = nullptr;
    // 首先在 part 里找
    chunk = find(part, _len);
    // 没找到的话在 free 里找
    if (chunk == nullptr) {
        printf("find(part, _len) done, null.\n");
        chunk = find(free, _len);
        printf("find(free, _len) done, null.\n");
    }
    // 也没有的话就申请新的物理页，并在 free 中重新查找
    if (chunk == nullptr) {
        // 申请成功才会继续执行
        if (alloc_pmm(_len) == true) {
            printf("alloc_pmm(_len) done.\n");
            chunk = find(free, _len);
            printf("find(free, _len) done.\n");
        }
    }
    // 如果到这里 chunk 还为 nullptr 说明空间不够了
    printf("find(size_t) done.\n");
    return chunk;
}

void SLAB::slab_cache_t::move(chunk_t *_node) {
    // 从当前链表中删除
    _node->prev->next = _node->next;
    _node->next->prev = _node->prev;
    // 移动到对应链表
    // 如果节点的 len 小于等于 len，说明没有多余空间
    if (_node->len <= len) {
        // 移动到 full
        full.push_back(_node);
        printf("move to full.\n");
    }
    // 如果节点的 len 大于 len，说明还有多余空间
    else if (_node->len > sizeof(chunk_t)) {
        // 移动到 part
        part.push_back(_node);
        printf("move to part.\n");
    }
    // 如果节点的 len 等于 alloc_pmm 分配的减去节点大小，说明这一页还没有使用
    else if (_node->len == len - sizeof(chunk_t)) {
        // 移动到 free
        free.push_back(_node);
        printf("move to free.\n");
    }
    return;
}

bool SLAB::slab_cache_t::alloc_pmm(size_t _len) {
    // 计算页数
    size_t pages = _len / COMMON::PAGE_SIZE;
    if (_len % COMMON::PAGE_SIZE != 0) {
        pages += 1;
    }
    // 申请
    chunk_t *new_node = (chunk_t *)PMM::alloc_pages(pages);
    // 不为空的话进行初始化
    if (new_node != nullptr) {
        // 初始化
        // 自身的地址
        new_node->addr = (void *)new_node;
        // 长度需要减去 chunk_t 的长度
        new_node->len = (pages * COMMON::PAGE_SIZE) - sizeof(chunk_t);
        // 链表指针
        new_node->prev = new_node;
        new_node->next = new_node;
        // 插入 free 节点
        free.push_back(new_node);
        // 返回
        return true;
    }
    // 如果为空则返回 false
    return false;
}

size_t SLAB::get_idx(size_t _len) {
    size_t res = 0;
    // _len 向上取整
    _len += _len - 1;
    while (1) {
        // 每次右移一位
        _len = _len >> 1;
        if (_len == 0) {
            // res 需要减去 SHIFT
            res -= SHIFT;
            break;
        }
        res++;
    }
    return res;
}

SLAB::SLAB(const void *_addr, size_t _len) : ALLOCATOR(_addr, _len) {
    name = (char *)"HEAP(SLAB) allocator";
    // 初始化 slab_cache
    for (size_t i = LEN64; i < LEN65536; i++) {
        slab_cache[i].len = MIN << i;
    }
    printf("%s init.\n", name);
    return;
}

SLAB::~SLAB(void) {
    return;
}

void *SLAB::alloc(size_t _len) {
    void *res = nullptr;
    // 分配时，首先确定需要分配的大小
    auto bytes = _len + sizeof(chunk_t);
    // 根据大小确定 slab_cache 索引
    auto idx = get_idx(bytes);
    // 寻找合适的 slab 节点
    chunk_t *chunk = slab_cache[idx].find(bytes);
    printf("slab_cache[0x%X].len: 0x%X\n", idx, slab_cache[idx].len);
    // 不为空的话计算地址
    if (chunk != nullptr) {
        printf("chunk->addr: 0x%X, chunk->len: 0x%X\n", chunk->addr,
               chunk->len);
        // 计算地址
        res = (uint8_t *)chunk->addr + sizeof(chunk_t);
    }
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

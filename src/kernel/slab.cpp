
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
    printf("new chunk\n");
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
    // 如果 this 指针为空，说明链表头节点为空
    if (this == nullptr) {
        // 为 this 赋值
        // this = _new_node;
    }
    _new_node->next = this;
    printf("this: 0x%p\n", this);
    // printf("prev: 0x%p\n", prev);
    // printf("next: 0x%p\n", next);
    printf("_new_node: 0x%p\n", _new_node);
    printf("_new_node->next: 0x%p\n", _new_node->next);
    printf("_new_node->prev: 0x%p\n", _new_node->prev);
    while (1)
        ;
    _new_node->prev = prev;
    prev->next      = _new_node;
    prev            = _new_node;
    return;
}

void SLAB::slab_cache_t::move(chunk_t *_node) {
    // 从当前链表中删除
    _node->prev->next = _node->next;
    _node->next->prev = _node->prev;
    // 移动到对应链表
    // 如果节点的 len 等于 alloc_pmm 分配的减去节点大小，说明这一页还没有使用
    if (_node->len == len - CHUNK_SIZE) {
        // 移动到 free
        free->push_back(_node);
        printf("move to free: 0x%X.\n", _node->addr);
    }
    // 如果节点的 len 大于 len，说明还有多余空间
    else if (_node->len > len) {
        // 移动到 part
        part->push_back(_node);
        printf("move to part: 0x%X.\n", _node->addr);
    }
    // 如果节点的 len 小于等于 len，说明没有多余空间
    else if (_node->len <= len) {
        // 移动到 full
        full->push_back(_node);
        printf("move to full: 0x%X.\n", _node->addr);
    }
    return;
}

SLAB::chunk_t *SLAB::slab_cache_t::alloc_pmm(size_t _len) {
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
        new_node->len = (pages * COMMON::PAGE_SIZE) - CHUNK_SIZE;
        // 链表指针
        new_node->prev = new_node;
        new_node->next = new_node;
        // 插入 free 节点
        free->push_back(new_node);
    }
    return new_node;
}

void SLAB::slab_cache_t::free_pmm(void) {
    // 遍历 free，符合条件的释放
    return;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(chunk_t *_which, size_t _len,
                                        bool _alloc) {
    chunk_t *res = nullptr;
    //  part/free 都是空
    if (part == nullptr && free == nullptr && _alloc == true) {
        // 申请新的内存
        res = alloc_pmm(_len);
    }
    // 如果 part 不为空
    if (part != nullptr) {
        // 在 part 中查找
        // res 指向移动到 full
        // 判断剩余空间是否大于
        // len+CHUNK_SIZE，如果大于等于则建立新的节点 小于的话不用新建
    }
    // 如果 part 为空或在 part 中没有找到
    if (res == nullptr) {
        // 在 free 中查找
        // res 指向的移动到 full
        // 判断剩余空间是否大于
        // len+CHUNK_SIZE，如果大于等于则建立新的节点 小于的话不用新建
    }
    return res;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(size_t _len) {
    printf("_len: 0x%X\n", _len);
    chunk_t *chunk = nullptr;
    // 首先在 part 里找
    chunk = find(part, _len, false);
    // 没找到的话在 free 里找
    if (chunk == nullptr) {
        printf("find(part, _len) done, null.\n");
        // 在 free 中寻找，如果没找到就申请新的物理内存
        chunk = find(free, _len, true);
        printf("find(free, _len) done, null.\n");
    }
    // 如果到这里 chunk 还为 nullptr 说明空间不够了
    assert(chunk != nullptr);
    printf("find(size_t) done.\n");
    return chunk;
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
            if (res < SHIFT) {
                res = 0;
            }
            else {
                res -= SHIFT;
            }
            break;
        }
        res++;
    }
    return res;
}

SLAB::SLAB(const void *_addr, size_t _len) : ALLOCATOR(_addr, _len) {
    name = (char *)"HEAP(SLAB) allocator";
    // 初始化 slab_cache
    for (size_t i = LEN256; i < LEN65536; i++) {
        slab_cache[i].len  = MIN << i;
        slab_cache[i].full = nullptr;
        slab_cache[i].part = nullptr;
        slab_cache[i].free = nullptr;
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
    auto bytes = _len + CHUNK_SIZE;
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
        res = (uint8_t *)chunk->addr + CHUNK_SIZE;
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

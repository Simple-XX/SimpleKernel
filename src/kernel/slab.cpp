
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
    addr = (void *)HEAD;
    len  = HEAD;
    prev = this;
    next = this;
    return;
}

SLAB::chunk_t::~chunk_t(void) {
    return;
}

size_t SLAB::chunk_t::size(void) {
    size_t   res = 0;
    chunk_t *tmp = this->next;
    while (tmp != this) {
        res++;
        tmp = tmp->next;
    }
    return res;
}

bool SLAB::chunk_t::operator==(const chunk_t &_node) {
    return addr == _node.addr && len == _node.len && prev == _node.prev &&
           next == _node.next;
}

bool SLAB::chunk_t::operator!=(const chunk_t &_node) {
    return addr != _node.addr || len != _node.len || prev != _node.prev ||
           next != _node.next;
}

// 由于是循环队列，相当于在头节点前面插入
void SLAB::chunk_t::push_back(chunk_t *_new_node) {
    _new_node->next = this;
    _new_node->prev = prev;
    prev->next      = _new_node;
    prev            = _new_node;
    return;
}

void SLAB::slab_cache_t::move(chunk_t &_list, chunk_t *_node) {
    // 从当前链表中删除
    _node->prev->next = _node->next;
    _node->next->prev = _node->prev;
    // 重置指针
    _node->prev = _node;
    _node->next = _node;
    // 插入新链表
    _list.push_back(_node);
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
        // 加入 free 链表
        free.push_back(new_node);
    }
    printf("alloc_pmm: 0x%X, pages: 0x%X, new_node: 0x%p\n", _len, pages,
           new_node);
    return new_node;
}

void SLAB::slab_cache_t::free_pmm(void) {
    // 遍历 free，符合条件的释放
    chunk_t *tmp   = free.next;
    size_t   bytes = 0;
    size_t   pages = 0;
    for (size_t i = 0; i < free.size(); i++) {
        bytes = tmp->len + CHUNK_SIZE;
        pages = bytes / COMMON::PAGE_SIZE;
        if (bytes % COMMON::PAGE_SIZE != 0) {
            pages += 1;
        }
        PMM::alloc_pages(tmp->addr, pages);
        tmp = tmp->next;
    }
    return;
}

void SLAB::slab_cache_t::split(chunk_t *_node, size_t _len) {
    // 处理新节点
    chunk_t *new_node = (chunk_t *)((uint8_t *)_node->addr + CHUNK_SIZE + len);
    new_node->addr    = new_node;
    // 剩余长度为原本的长度减去要分配给 _node 的长度，减去新节点的 chunk 大小
    new_node->len = _node->len - _len - CHUNK_SIZE;
    // 手动初始化节点
    new_node->prev = new_node;
    new_node->next = new_node;
    // 判断剩余空间是否可以容纳至少一个节点，即大于等于  len+CHUNK_SIZE
    // 如果大于等于则建立新的节点小于的话不用新建
    // 这里只有 len 是因为 chunk 的大小并不包括在 chunk->len 中，
    // 前面几行代码已经计算过了
    if (new_node->len >= len) {
        // 新的节点必然属于 part 链表
        part.push_back(new_node);
    }
    printf("new_node->addr: 0x%X, new_node->len: 0x%X\n", new_node->addr,
           new_node->len);
    // 设置旧节点
    _node->len = _len;
    return;
}

void SLAB::slab_cache_t::merge(void) {
    // 如果节点少于两个，不需要合并
    if (part.size() < 2) {
        return;
    }
    // 遍历 part 所有节点，如果有 addr 连续的，则合并
    // 直观看得 O(N^2)，有啥效率高的？
    chunk_t *tmp1 = this->part.next;
    chunk_t *tmp2 = nullptr;
    void *   addr = nullptr;
    // 前面的节点
    for (size_t i = 0; i < part.size(); i++) {
        addr = (uint8_t *)tmp1->addr + CHUNK_SIZE + tmp1->len;
        // 后面的节点
        for (size_t j = i + 1; j < part.size(); j++) {
            // 前面节点的结束地址是后面节点的开始地址
            if (addr == tmp2->addr) {
                // chunk 大小
                tmp1->len += CHUNK_SIZE;
                // 数据长度
                tmp1->len += tmp2->len;
                // 从当前链表中删除
                tmp2->prev->next = tmp2->next;
                tmp2->next->prev = tmp2->prev;
            }
            tmp2 = tmp1->next;
        }
        tmp1 = tmp1->next;
    }

    // 后面的合并前面的

    // 如果长度符合，移动到 free
    tmp1 = this->part.next;
    for (size_t i = 0; i < part.size(); i++) {
        if (tmp1->len >= len - CHUNK_SIZE) {
            // 移动到 free
            move(free, tmp1);
        }
        tmp1 = tmp1->next;
    }
    // 如果有可以释放的则释放
    free_pmm();
    return;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(chunk_t &_which, size_t _len,
                                        bool _alloc) {
    chunk_t *res = nullptr;
    // 在 _which 中查找，直接遍历即可
    chunk_t *tmp = _which.next;
    // 遍历节点
    for (size_t i = 0; i < _which.size(); i++) {
        // 如果 tmp 节点的长度大于等于 _len
        if (tmp->len >= _len) {
            // 更新 res
            res = tmp;
            printf("000 res->addr: 0x%X, res->len: 0x%X\n", res->addr,
                   res->len);
            // 跳出循环
            break;
        }
        // 不符合要求的话就到下一个节点
        tmp = tmp->next;
    }
    // 如果 res 不为空
    if (res != nullptr) {
        printf("aaaa res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
        // res 从旧节点分离出来后剩余的部分
        split(res, _len);
        printf("bbbb res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
        // res 指向的移动到 full
        move(full, res);
        printf("cccc res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
    }
    // 如果 res 为空，说明在 _which 链表中没有找到合适的节点
    // 如果同时 _alloc 成立
    else if (res == nullptr && _alloc == true) {
        // 申请新的空间
        res = alloc_pmm(_len);
        printf("dddd res->addr: 0x%X, res->len: 0x%X\n", res->addr, res->len);
        // 申请成功
        if (res != nullptr) {
            // 剩余部分
            split(res, _len);
            printf("eeee res->addr: 0x%X, res->len: 0x%X\n", res->addr,
                   res->len);
        }
    }
    return res;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(size_t _len) {
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

void SLAB::slab_cache_t::remove(chunk_t *_node) {
    // 将 _node 移动到 part 即可
    move(part, _node);
    // merge 会处理节点合并的情况
    merge();
    return;
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
    auto bytes = _len + CHUNK_SIZE;
    // 大小不能超过 65536B
    if (bytes <= MIN << LEN65536) {
        // 根据大小确定 slab_cache 索引
        auto idx = get_idx(bytes);
        // 寻找合适的 slab 节点
        chunk_t *chunk = slab_cache[idx].find(bytes);
        // 不为空的话计算地址
        if (chunk != nullptr) {
            printf("chunk->addr: 0x%X, chunk->len: 0x%X\n", chunk->addr,
                   chunk->len);
            // 计算地址
            res = (uint8_t *)chunk->addr + CHUNK_SIZE;
        }
    }
    // 返回
    return res;
}

bool SLAB::alloc(void *, size_t) {
    return true;
}

void SLAB::free(void *_addr, size_t) {
    printf("_addr: 0x%X\n", _addr);
    if (_addr == nullptr) {
        return;
    }
    // 要释放一个 chunk
    // 1. 计算 chunk 地址
    chunk_t *chunk = (chunk_t *)((uint8_t *)_addr - CHUNK_SIZE);
    printf("free chunk: 0x%X\n", chunk);
    // 2. 计算所属 slab_cache 索引
    printf("free chunk->len: 0x%X\n", chunk->len);
    assert(chunk->len != 0);
    auto idx = get_idx(chunk->len);
    // 3. 调用对应的 remove 函数
    // slab_cache[idx].remove(chunk);
    return;
}

size_t SLAB::get_used_count(void) const {
    return 0;
}

size_t SLAB::get_free_count(void) const {
    return 0;
}

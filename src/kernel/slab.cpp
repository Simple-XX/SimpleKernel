
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "list_tmp.hpp"
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

SLAB::chunk_t &SLAB::chunk_t::operator[](size_t _idx) {
    // 判断越界
    assert(_idx < size());
    chunk_t *res  = nullptr;
    chunk_t *head = this;
    // 找到头节点
    for (size_t i = 0; i < size(); i++) {
        if (head->addr == (void *)HEAD && head->len == HEAD) {
            // res 指向 head 的下一个，即 [0]
            res = head->next;
            break;
        }
        head = head->next;
    }
    // 返回第 _idx 个节点
    for (size_t i = 0; i < _idx; i++) {
        res = res->next;
    }
    // res 必不为空
    assert(res != nullptr);
    return *res;
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
    return new_node;
}

void SLAB::slab_cache_t::free_pmm(void) {
    size_t pages = 0;
    // 遍历 free 链表
    for (size_t i = 0; i < free.size(); i++) {
        chunk_t &tmp = free[i];
        pages        = (tmp.len + CHUNK_SIZE) / COMMON::PAGE_SIZE;
        // 必须是整数个页
        assert(((tmp.len + CHUNK_SIZE) % COMMON::PAGE_SIZE) == 0);
        PMM::free_pages(tmp.addr, pages);
        // 删除节点
        tmp.prev->next = tmp.next;
        tmp.next->prev = tmp.prev;
    }
    // 释放完后 free 链表项应该为 0
    assert(free.size() == 0);
    return;
}

void SLAB::slab_cache_t::split(chunk_t *_node, size_t _len) {
    // 处理新节点
    // 新节点地址为原本地址+chunk大小+要分配出去的长度
    chunk_t *new_node = (chunk_t *)((uint8_t *)_node->addr + CHUNK_SIZE + _len);
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
    // 设置旧节点
    _node->len = _len;
    // 旧节点移动到 full
    move(full, _node);
    return;
}

// TODO: 优化算法
void SLAB::slab_cache_t::merge(void) {
    // 如果节点少于两个，不需要合并
    if (part.size() < 2) {
        return;
    }
    // 合并的条件
    // node1->addr+CHUNK_SIZE+node1->len==node2->addr
    // 暴力遍历
    // 外层循环
    for (size_t i = 0; i < part.size(); i++) {
        chunk_t &tmp  = part[i];
        chunk_t *tmp2 = tmp.next;
        // 内层循环
        while (*tmp2 != tmp) {
            // 如果符合条件
            if ((uint8_t *)tmp.addr + CHUNK_SIZE + tmp.len == tmp2->addr) {
                // 进行合并
                // 加上 tmp2 的 chunk 长度
                tmp.len += CHUNK_SIZE;
                // 加上 tmp2 的 len 长度
                tmp.len += tmp2->len;
                // 删除 tmp2
                tmp2->prev->next = tmp2->next;
                tmp2->next->prev = tmp2->prev;
                break;
            }
            tmp2 = tmp2->next;
        }
    }
    // 遍历查找可以移动到 free 链表的
    // 如果 part 长度等于 len-chunnk 大小
    for (size_t i = 0; i < part.size(); i++) {
        chunk_t &tmp = part[i];
        // 节点 len + chunk 长度对页大小取余，如果为零说明有整数页没有被使用
        if (((tmp.len + CHUNK_SIZE) % COMMON::PAGE_SIZE) == 0) {
            // 移动到 free
            move(free, &tmp);
        }
    }
    // 寻找可以释放的节点进行释放
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
            // 跳出循环
            break;
        }
        // 不符合要求的话就到下一个节点
        tmp = tmp->next;
    }
    // 如果 res 不为空
    if (res != nullptr) {
        // res 从旧节点分离出来后剩余的部分
        split(res, _len);
        // res 指向的移动到 full
        move(full, res);
    }
    // 如果 res 为空，说明在 _which 链表中没有找到合适的节点
    // 如果同时 _alloc 成立
    else if (res == nullptr && _alloc == true) {
        // 申请新的空间
        res = alloc_pmm(_len);
        // 申请成功
        if (res != nullptr) {
            // 剩余部分
            split(res, _len);
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
        // 在 free 中寻找，如果没找到就申请新的物理内存
        chunk = find(free, _len, true);
    }
    // 如果到这里 chunk 还为 nullptr 说明空间不够了
    assert(chunk != nullptr);
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
    allocator_name = (char *)"HEAP(SLAB) allocator";
    // 初始化 slab_cache
    for (size_t i = LEN256; i < LEN65536; i++) {
        slab_cache[i].len = MIN << i;
    }
    printf("%s init.\n", allocator_name);
    return;
}

SLAB::~SLAB(void) {
    return;
}

void *SLAB::alloc(size_t _len) {
    printf("alloc _len: 0x%p\n", _len);
    void *res = nullptr;
    // 分配时，首先确定需要分配的大小
    // 大小不能超过 65536B
    if (_len <= MIN << LEN65536) {
        // 根据大小确定 slab_cache 索引
        auto idx = get_idx(_len);
        // 寻找合适的 slab 节点
        chunk_t *chunk = slab_cache[idx].find(_len);
        printf("alloc find cache: 0x%p\n", chunk);
        printf("alloc find cache chunk->addr: 0x%p\n", chunk->addr);
        // 不为空的话计算地址
        if (chunk != nullptr) {
            // 计算地址
            res = (uint8_t *)chunk->addr + CHUNK_SIZE;
            printf("alloc find cache: 0x%p\n", res);
        }
#define DEBUG
#ifdef DEBUG
        std::cout << slab_cache[idx];
#undef DEBUG
#endif
    }
    // 返回
    printf("alloc return: 0x%p\n", res);
    return res;
}

bool SLAB::alloc(void *, size_t) {
    return true;
}

void SLAB::free(void *_addr, size_t) {
    if (_addr == nullptr) {
        return;
    }
    // 要释放一个 chunk
    // 1. 计算 chunk 地址
    chunk_t *chunk = (chunk_t *)((uint8_t *)_addr - CHUNK_SIZE);
    // 2. 计算所属 slab_cache 索引
    assert(chunk->len != 0);
    auto idx = get_idx(chunk->len);
    // 3. 调用对应的 remove 函数
    slab_cache[idx].remove(chunk);
// #define DEBUG
#ifdef DEBUG
    std::cout << slab_cache[idx];
#undef DEBUG
#endif
    return;
}

size_t SLAB::get_used_count(void) const {
    return 0;
}

size_t SLAB::get_free_count(void) const {
    return 0;
}

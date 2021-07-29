
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "pmm.h"
#include "vmm.h"
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
    chunk_t *res = nullptr;
    // 找到头节点
    chunk_t *tmp = this;
    while (tmp->next != this) {
        if (tmp->addr == (void *)HEAD && tmp->len == HEAD) {
            res = tmp;
            break;
        }
        tmp = tmp->next;
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
    // 如果没有映射则进行映射
    void *tmp = nullptr;
    for (size_t i = 0; i < pages; i++) {
        // 地址=初始地址+页数偏移
        tmp = (uint8_t *)new_node + i * COMMON::PAGE_SIZE;
        if (VMM::get_mmap(VMM::get_pgd(), tmp, nullptr) == false) {
            VMM::mmap(VMM::get_pgd(), (void *)new_node, (void *)new_node,
                      VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
        }
        // 已经映射的情况是不应该出现的
        else {
            assert(0);
        }
    }
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
    chunk_t *tmp = free.next;
    while (tmp != &free) {
        pages = (tmp->len + CHUNK_SIZE) / COMMON::PAGE_SIZE;
        // 必须是整数个页
        assert(((tmp->len + CHUNK_SIZE) % COMMON::PAGE_SIZE) == 0);
        PMM::free_pages(tmp->addr, pages);
        // 删除节点
        tmp->prev->next = tmp->next;
        tmp->next->prev = tmp->prev;
        // 取消映射后无法访问 tmp，所以提前保存
        auto tmp_next = tmp->next;
        // 取消映射
        // 因为每次只能取消映射 1 页，所以需要循环
        void *tmp_addr = nullptr;
        for (size_t i = 0; i < pages; i++) {
            tmp_addr = (uint8_t *)tmp->addr + i * COMMON::PAGE_SIZE;
            VMM::unmmap(VMM::get_pgd(), tmp_addr);
        }
        // 迭代
        tmp = tmp_next;
    }
    // 释放完后 free 链表项应该为 0
    assert(free.size() == 0);
    return;
}

void SLAB::slab_cache_t::split(chunk_t *_node, size_t _len) {
    // 记录原大小
    size_t old_len = _node->len;
    // 更新旧节点
    _node->len = _len;
    // 旧节点移动到 full
    move(full, _node);
    // 原长度大于要分配的长度+新 chunk 长度
    // 不能等于，等于的话相当于新节点的 len 为 0
    if (old_len > _len + CHUNK_SIZE) {
        // 处理新节点
        // 新节点地址为原本地址+chunk大小+要分配出去的长度
        chunk_t *new_node =
            (chunk_t *)((uint8_t *)_node->addr + CHUNK_SIZE + _len);
        new_node->addr = new_node;
        // 剩余长度为原本的长度减去要分配给 _node 的长度，减去新节点的 chunk
        // 大小
        new_node->len = old_len - _len - CHUNK_SIZE;
        // 手动初始化节点
        new_node->prev = new_node;
        new_node->next = new_node;
        // 判断剩余空间是否可以容纳至少一个节点，即大于等于  len+CHUNK_SIZE
        // 如果大于等于则建立新的节点，小于的话不用新建
        // 这里只有 len 是因为 chunk 的大小并不包括在 chunk->len
        // 中， 前面几行代码已经计算过了
        if (new_node->len > len) {
            // 新的节点必然属于 part 链表
            part.push_back(new_node);
        }
    }
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
    chunk_t *chunk = part.next;
    chunk_t *tmp   = chunk->next;
    // 外层循环
    while (chunk != &part) {
        // 内层循环
        while (chunk != tmp) {
            // 如果符合条件
            if ((uint8_t *)chunk->addr + CHUNK_SIZE + chunk->len == tmp->addr) {
                // 进行合并
                // 加上 tmp 的 chunk 长度
                chunk->len += CHUNK_SIZE;
                // 加上 tmp 的 len 长度
                chunk->len += tmp->len;
                // 删除 tmp
                tmp->prev->next = tmp->next;
                tmp->next->prev = tmp->prev;
                break;
            }
            tmp = tmp->next;
        }
        chunk = chunk->next;
    }

    // 遍历查找可以移动到 free 链表的
    // 如果 part 长度等于 len-chunnk 大小
    tmp = part.next;
    while (tmp != &part) {
        // 节点 len + chunk 长度对页大小取余，如果为零说明有整数页没有被使用
        if (((tmp->len + CHUNK_SIZE) % COMMON::PAGE_SIZE) == 0) {
            // 移动到 free
            move(free, tmp);
            // 因为 tmp 已经被修改了，所以重新赋值
            tmp = part.next;
        }
        else {
            // 没有被修改，直接指向 next
            tmp = tmp->next;
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
    while (tmp != &_which) {
        // 如果 tmp 节点的长度大于等于 _len
        if (tmp->len >= _len) {
            // 更新 res
            res = tmp;
            // 跳出循环
            break;
        }
        tmp = tmp->next;
    }
    // 如果 res 为空，说明在 _which 链表中没有找到合适的节点
    // 如果同时 _alloc 成立
    if (res == nullptr && _alloc == true) {
        // 申请新的空间
        res = alloc_pmm(_len);
    }
    // 如果 res 不为空
    if (res != nullptr) {
        // 对 res 进行切割，res 加入 full，剩余部分进入 part
        split(res, _len);
    }
    return res;
}

SLAB::chunk_t *SLAB::slab_cache_t::find(size_t _len) {
    chunk_t *chunk = nullptr;
    // 在 part 里找，如果没有找到允许申请新的空间
    chunk = find(part, _len, true);
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

SLAB::SLAB(const char *_name, const void *_addr, size_t _len)
    : ALLOCATOR(_name, _addr, _len) {
    // 初始化 slab_cache
    for (size_t i = LEN256; i < LEN65536; i++) {
        slab_cache[i].len = MIN << i;
    }
    printf("%s: 0x%p(0x%p bytes) init.\n", name, allocator_start_addr,
           allocator_length);
    return;
}

SLAB::~SLAB(void) {
    printf("%s finit.\n", name);
    return;
}

void *SLAB::alloc(size_t _len) {
    void *res = nullptr;
    // 分配时，首先确定需要分配的大小
    // _len 为零直接返回
    // 大小不能超过 65536B
    if (_len > 0 && _len <= MIN << LEN65536) {
        // _len 按照 8bytes 对齐
        _len = COMMON::ALIGN(_len, 8);
        // 根据大小确定 slab_cache 索引
        auto idx = get_idx(_len);
        // 寻找合适的 slab 节点
        chunk_t *chunk = slab_cache[idx].find(_len);
        // 不为空的话计算地址
        if (chunk != nullptr) {
            assert(chunk == chunk->addr);
            // 计算地址
            res = (uint8_t *)chunk->addr + CHUNK_SIZE;
        }
// #define DEBUG
#ifdef DEBUG
        printf("slab alloc\n");
        std::cout << slab_cache[idx];
#undef DEBUG
#endif
    }
    // 更新统计数据
    allocator_used_count += _len;
    // 返回
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
    assert(chunk == chunk->addr);
    // 2. 计算所属 slab_cache 索引
    assert(chunk->len != 0);
    auto idx = get_idx(chunk->len);
    // 3. 调用对应的 remove 函数
    slab_cache[idx].remove(chunk);
// #define DEBUG
#ifdef DEBUG
    printf("slab free\n");
    std::cout << slab_cache[idx];
#undef DEBUG
#endif
    // 更新统计数据
    allocator_used_count -= _len;
    return;
}

size_t SLAB::get_used_count(void) const {
    return allocator_used_count;
}

size_t SLAB::get_free_count(void) const {
    return 0;
}


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

bool FIRSTFIT::ff_entry_t::operator==(const ff_entry_t &_ff) {
    if (addr == _ff.addr && npages == _ff.npages && used == _ff.used) {
        return true;
    }
    else {
        return false;
    }
}

FIRSTFIT::FIRSTFIT(const void *_addr, size_t _len)
    : ALLOCATOR(_addr, _len), list(const_cast<void *>(COMMON::PMM_INFO_START),
                                   const_cast<void *>(COMMON::PMM_INFO_END)) {
    name           = (char *)"PMM(First Fit) allocator";
    list[0].addr   = const_cast<void *>(_addr);
    list[0].npages = _len;
    list[0].used   = false;
    printf("%s init.\n", name);
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    return;
}

void *FIRSTFIT::alloc(size_t _len) {
    void *res_addr = nullptr;
    // 遍历链表
    // 因为位置不确定，所以得遍历全部数据
    for (size_t i = 0; i < list.nodes_len; i++) {
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
            // 更新统计数据
            free_pages_count -= _len;
            used_pages_count += _len;
            // TODO: 合并：状态相同的几段内存可以统一进行管理
            // 返回
            printf("list.size: 0x%X\n", list.size);
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

// md 好麻烦
// 先推一下几种情况
// 直接找到
// 1. 01-0: 需要将 - 分割出来，同时 01 合并. 0010
// 2. 010: 010 合并. 000
// 被合并
// 3. -1-0: 需要将前后分割出来，不需要合并. 1010
// 4. -10: 需要将 -1 分割，10 合并. 100
// 同时要避免内存回卷
// 这尼玛写死了
// 得想个高效的方法
// 在这里不管能不能合并，统统都执行一遍 merge 函数，在函数里判断
// 设计太傻逼了，重新想想咋写再来

ff_entry_t &FIRSTFIT::merge(ff_entry_t &_first, ff_entry_t &_second) {
    // 记录返回值
    ff_entry_t res;
    // 记录在链表中的位置
    // 如果 used 标识相同
    if (_first.used == _second.used) {

        merge;
    }
    // 删除被合并节点
    // 返回合并节点
    return res;
}

void FIRSTFIT::free(void *_addr, size_t _len) {
    // 释放需要考虑前后的合并
    // 首先定位到对应的 node
    // 因为位置不确定，所以得遍历全部数据
    for (size_t i = 0; i < list.nodes_len; i++) {
        // 已使用
        if (list[i].used == true) {
            // 下面开始分情况处理
            if (list[i].addr == _addr) {
                // 1.
                if (list.next(list[i]).used == true) {
                    return;
                }
                // 2.
                else if (list.next(list[i]).used == false) {
                    ;
                }
            }
            else if (list[i].addr <= _addr &&
                     ((uint8_t *)list[i].addr + _len * COMMON::PAGE_SIZE) >=
                         _addr) {
                // 3.
                if (list.next(list[i]).used == true) {
                    ;
                }
                // 4.
                if (list.next(list[i]).used == false) {
                    ;
                }
            }
        }
        // 执行到这里说明符合要求的内存不存在，报错
        err("FREE ERROR!\n");
        assert(0);
        return;
    }

    size_t FIRSTFIT::get_used_pages_count(void) const {
        return used_pages_count;
    }

    size_t FIRSTFIT::get_free_pages_count(void) const {
        return free_pages_count;
    }

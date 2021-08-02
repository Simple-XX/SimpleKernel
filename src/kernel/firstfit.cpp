
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

void FIRSTFIT::set(size_t _idx) {
    map[_idx >> SHIFT] |= (uintptr_t)1 << (_idx & MASK);
    return;
}

void FIRSTFIT::clr(size_t _idx) {
    map[_idx >> SHIFT] &= ~((uintptr_t)1 << (_idx & MASK));
    return;
}

bool FIRSTFIT::test(size_t _idx) {
    return map[_idx >> SHIFT] & ((uintptr_t)1 << (_idx & MASK));
}

size_t FIRSTFIT::find_len(size_t _len, bool _val) {
    size_t count = 0;
    size_t idx   = 0;
    // 遍历位图
    for (uintptr_t i = 0; i < allocator_length; i++) {
        if (test(i) != _val) {
            count = 0;
            idx   = i + 1;
        }
        else {
            count++;
        }
        if (count == _len) {
            return idx;
        }
    }
    return ~(size_t)0;
}

FIRSTFIT::FIRSTFIT(const char *_name, uintptr_t _addr, size_t _len)
    : ALLOCATOR(_name, _addr, _len) {
    // 所有清零
    bzero(map, sizeof(map));
    info("%s: 0x%p(0x%X pages) init.\n", name, allocator_start_addr,
         allocator_length);
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    info("%s finit.\n", name);
    return;
}

uintptr_t FIRSTFIT::alloc(size_t _len) {
    uintptr_t res_addr = 0;
    // 在位图中寻找连续 _len 的位置
    size_t idx = find_len(_len, false);
    // 如果为 ~0 说明未找到
    if (idx == ~(size_t)0) {
        // err("NO ENOUGH MEM.\n");
        return res_addr;
    }
    // 遍历区域
    for (auto i = idx; i < idx + _len; i++) {
        // 置位，说明已使用
        set(i);
    }
    // 计算实际地址
    // 分配器起始地址+页长度*第几页
    res_addr = allocator_start_addr + (COMMON::PAGE_SIZE * idx);
    // 更新统计信息
    allocator_free_count -= _len;
    allocator_used_count += _len;
    return res_addr;
}

bool FIRSTFIT::alloc(uintptr_t _addr, size_t _len) {
    // _addr 不在管理范围内
    if ((_addr < allocator_start_addr) ||
        (_addr >=
         allocator_start_addr + allocator_length * COMMON::PAGE_SIZE)) {
        return false;
    }
    // 计算 _addr 在 map 中的索引
    size_t idx = (_addr - allocator_start_addr) / COMMON::PAGE_SIZE;
    // 遍历
    for (auto i = idx; i < idx + _len; i++) {
        // 如果在范围内有已经分配的内存，返回 false
        if (test(i) == true) {
            return false;
        }
    }
    // 到这里说明范围内没有已使用内存
    // 再次遍历
    for (auto i = idx; i < idx + _len; i++) {
        // 置位
        set(i);
    }
    // 更新统计信息
    allocator_free_count -= _len;
    allocator_used_count += _len;
    return true;
}

void FIRSTFIT::free(uintptr_t _addr, size_t _len) {
    // _addr 不在管理范围内
    if ((_addr < allocator_start_addr) ||
        (_addr >=
         allocator_start_addr + allocator_length * COMMON::PAGE_SIZE)) {
        return;
    }
    // 计算 _addr 在 map 中的索引
    size_t idx = (_addr - allocator_start_addr) / COMMON::PAGE_SIZE;
    for (auto i = idx; i < idx + _len; i++) {
        clr(i);
    }
    // 更新统计信息
    allocator_free_count += _len;
    allocator_used_count -= _len;
    return;
}

size_t FIRSTFIT::get_used_count(void) const {
    return allocator_used_count;
}

size_t FIRSTFIT::get_free_count(void) const {
    return allocator_free_count;
}

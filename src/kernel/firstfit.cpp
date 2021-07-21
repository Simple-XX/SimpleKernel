
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

void FIRSTFIT::set(uint64_t _idx) {
    map[_idx >> SHIFT] |= (uint64_t)1 << (_idx & MASK);
    return;
}

void FIRSTFIT::clr(uint64_t _idx) {
    map[_idx >> SHIFT] &= ~((uint64_t)1 << (_idx & MASK));
    return;
}

bool FIRSTFIT::test(uint64_t _idx) {
    return map[_idx >> SHIFT] & ((uint64_t)1 << (_idx & MASK));
}

uint64_t FIRSTFIT::find_len(uint64_t _len, bool _val) {
    uint64_t count = 0;
    uint64_t idx   = 0;
    // 遍历位图
    for (uint64_t i = 0; i < (COMMON::PMM_SIZE / COMMON::PAGE_SIZE); i++) {
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
    return ~(uint64_t)0;
}

FIRSTFIT::FIRSTFIT(const char *_name, const void *_addr, size_t _len)
    : ALLOCATOR(_name, _addr, _len) {
    // 所有清零
    printf("__WORDSIZE: %d, %d\n", __WORDSIZE, BITS_PER_WORD);
    memset(map, 0, sizeof(map));
    printf("%s: 0x%p(0x%X pages) init.\n", name, allocator_start_addr,
           allocator_length);
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    printf("%s finit.\n", name);
    return;
}

void *FIRSTFIT::alloc(size_t _len) {
    void *res_addr = nullptr;
    // 在位图中寻找连续 _len 的位置
    uint64_t idx = find_len(_len, false);
    // 如果为 ~0 说明未找到
    if (idx == ~(uint64_t)0) {
        // err("NO ENOUGH MEM.\n");
        return nullptr;
    }
    // 遍历区域
    for (auto i = idx; i < idx + _len; i++) {
        // 置位，说明已使用
        set(i);
    }
    // 计算实际地址
    // 分配器起始地址+页长度*第几页
    res_addr =
        (void *)((uint8_t *)allocator_start_addr + (COMMON::PAGE_SIZE * idx));
    // 更新统计信息
    allocator_free_count -= _len;
    allocator_used_count += _len;
    return res_addr;
}

bool FIRSTFIT::alloc(void *_addr, size_t _len) {
    // 计算 _addr 在 map 中的索引
    uint64_t idx = ((uint8_t *)_addr - (uint8_t *)allocator_start_addr) /
                   COMMON::PAGE_SIZE;
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

void FIRSTFIT::free(void *_addr, size_t _len) {
    uint64_t idx = ((uint8_t *)_addr - (uint8_t *)allocator_start_addr) /
                   COMMON::PAGE_SIZE;
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

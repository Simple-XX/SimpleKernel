
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

FIRSTFIT::FIRSTFIT(const void *_addr, size_t _len) : ALLOCATOR(_addr, _len) {
    name = (char *)"PMM(First Fit) allocator";
    memset(map, 0, sizeof(map));
    printf("%s init.\n", name);
    return;
}

FIRSTFIT::~FIRSTFIT(void) {
    return;
}

void *FIRSTFIT::alloc(size_t _len) {
    void *res_addr = nullptr;
    // 在位图中寻找连续 _len 的位置
    uint64_t idx = find_len(_len, false);
    if (idx == ~(uint64_t)0) {
        // err("NO ENOUGH MEM.\n");
        return nullptr;
    }
    for (auto i = idx; i < idx + _len; i++) {
        set(i);
    }
    res_addr = (void *)((uint8_t *)addr + (COMMON::PAGE_SIZE * idx));
    free_pages_count -= _len;
    used_pages_count += _len;
    return res_addr;
}

bool FIRSTFIT::alloc(void *_addr, size_t _len) {
    uint64_t idx = ((uint8_t *)_addr - (uint8_t *)addr) / COMMON::PAGE_SIZE;
    for (auto i = idx; i < idx + _len; i++) {
        if (test(i) == true) {
            return false;
        }
    }
    for (auto i = idx; i < idx + _len; i++) {
        set(i);
    }
    free_pages_count -= _len;
    used_pages_count += _len;
    return true;
}

void FIRSTFIT::free(void *_addr, size_t _len) {
    uint64_t idx = ((uint8_t *)_addr - (uint8_t *)addr) / COMMON::PAGE_SIZE;
    for (auto i = idx; i < idx + _len; i++) {
        clr(i);
    }
    free_pages_count += _len;
    used_pages_count -= _len;
    return;
}

size_t FIRSTFIT::get_used_pages_count(void) const {
    return used_pages_count;
}

size_t FIRSTFIT::get_free_pages_count(void) const {
    return free_pages_count;
}


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// allocator.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdint.h"
#include "assert.h"
#include "common.h"
#include "allocator.h"

ALLOCATOR::ALLOCATOR(const char *_name, const void *_addr, size_t _len) {
    // 默认名字
    name                 = _name;
    allocator_start_addr = const_cast<void *>(_addr);
    allocator_length     = _len;
    // 初始状态下所有页都未使用
    allocator_free_count = allocator_length;
    allocator_used_count = 0;
    return;
}

ALLOCATOR::~ALLOCATOR(void) {
    return;
}

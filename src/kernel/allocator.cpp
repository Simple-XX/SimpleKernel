
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// allocator.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdint.h"
#include "assert.h"
#include "common.h"
#include "allocator.h"

ALLOCATOR::ALLOCATOR(const void *_addr, size_t _len) {
    name       = (char *)"ALLOCATOR: no init";
    addr       = const_cast<void *>(_addr);
    length     = _len;
    free_count = length;
    used_count = 0;
    return;
}

ALLOCATOR::~ALLOCATOR(void) {
    return;
}

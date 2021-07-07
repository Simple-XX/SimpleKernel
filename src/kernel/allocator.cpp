
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
    name             = (char *)"ALLOCATOR: no init";
    addr             = const_cast<void *>(_addr);
    length           = _len;
    pages_count      = length / COMMON::PAGE_SIZE;
    pages_free_count = pages_count;
    pages_used_count = 0;
    return;
}

ALLOCATOR::~ALLOCATOR(void) {
    return;
}

bool ALLOCATOR::init(void) {
    assert(0);
    return false;
}

void *ALLOCATOR::alloc(void) {
    assert(0);
    return nullptr;
}

bool ALLOCATOR::alloc(void *_addr) {
    (void)_addr;
    assert(0);
    return false;
}

void ALLOCATOR::free(void *_addr) {
    (void)_addr;
    assert(0);
    return;
}

size_t ALLOCATOR::get_used_pages_count(void) const {
    assert(0);
    return 0;
}

size_t ALLOCATOR::get_free_pages_count(void) const {
    assert(0);
    return 0;
}

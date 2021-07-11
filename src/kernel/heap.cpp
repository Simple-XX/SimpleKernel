
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "cpu.hpp"
#include "common.h"
#include "heap.h"

HEAP::HEAP(void) {
    return;
}

HEAP::~HEAP(void) {
    return;
}

bool HEAP::init(const void *_addr, size_t _len) {
    start  = _addr;
    length = _len;
    static SLAB slab_allocator(start, length);
    allocator = (ALLOCATOR *)&slab_allocator;
    printf("heap_init\n");
    return 0;
}

void *HEAP::malloc(size_t _byte) {
    return allocator->alloc(_byte);
}

void HEAP::free(void *_addr) {
    // 堆不需要 _len 参数
    allocator->free(_addr, 0);
    return;
}

size_t HEAP::get_total(void) {
    return 0;
}

size_t HEAP::get_block(void) {
    return 0;
}

size_t HEAP::get_free(void) {
    return allocator->get_free_count();
}

extern "C" void *malloc(size_t _size) {
    return heap.malloc(_size);
}

extern "C" void free(void *_p) {
    heap.free(_p);
    return;
}

HEAP heap;

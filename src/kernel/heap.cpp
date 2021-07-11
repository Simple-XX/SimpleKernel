
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

void *HEAP::malloc(size_t byte) {
    return allocator->alloc(byte);
}

void HEAP::free(void *addr) {
    // 堆不需要 _len 参数
    allocator->free(addr, 0);
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

extern "C" void *malloc(size_t size) {
    return heap.malloc(size);
}

extern "C" void free(void *ptr) {
    heap.free(ptr);
    return;
}

HEAP heap;

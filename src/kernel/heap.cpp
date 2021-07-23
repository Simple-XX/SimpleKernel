
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "heap.h"

HEAP::HEAP(void) {
    return;
}

HEAP::~HEAP(void) {
    return;
}

bool HEAP::init(void) {
    static SLAB slab_allocator("SLAB Allocator", 0, 0);
    allocator = (ALLOCATOR *)&slab_allocator;
    printf("heap init.\n");
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

extern "C" void *malloc(size_t _size) {
    return heap.malloc(_size);
}

extern "C" void free(void *_p) {
    heap.free(_p);
    return;
}

HEAP heap;

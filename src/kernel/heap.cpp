
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "cpu.hpp"
#include "common.h"
#include "heap.h"

ALLOCATOR *HEAP::allocator = nullptr;

HEAP::HEAP(void) {
    static SLAB slab_allocator(start, length);
    allocator = (ALLOCATOR *)&slab_allocator;
    return;
}

HEAP::~HEAP(void) {
    return;
}

bool HEAP::init(void) {
    printf("heap_init\n");
    return 0;
}

void *HEAP::malloc(size_t byte) {
    return manage.alloc(byte);
}

void HEAP::free(void *addr) {
    manage.free(addr);
    return;
}

size_t HEAP::get_total(void) {
    return manage.get_total();
}

size_t HEAP::get_block(void) {
    return manage.get_block();
}

size_t HEAP::get_free(void) {
    return manage.get_free();
}

extern "C" void *malloc(size_t size) {
    return heap.malloc(size);
}

extern "C" void free(void *ptr) {
    heap.free(ptr);
    return;
}

HEAP heap;

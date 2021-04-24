
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "cpu.hpp"
#include "common.h"
#include "heap.h"

SLAB HEAP::manage;

HEAP::HEAP(void) : name("SLAB") {
    return;
}

HEAP::~HEAP(void) {
    return;
}

int32_t HEAP::init(void) {
    manage_init();
    printf("heap_init\n");
    return 0;
}

int32_t HEAP::manage_init(void) {
    manage.init(COMMON::KERNEL_END_4K, HEAP_SIZE);
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

void *operator new(size_t size) {
    return malloc(size);
}

void *operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void *p) {
    free(p);
}

void operator delete(void *p, size_t size __attribute__((unused))) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

void operator delete[](void *p, size_t size __attribute__((unused))) {
    free(p);
}

void *operator new(size_t, void *p) throw() {
    return p;
}
void *operator new[](size_t, void *p) throw() {
    return p;
}

void operator delete(void *, void *) throw() {
    return;
};

void operator delete[](void *, void *) throw() {
    return;
};

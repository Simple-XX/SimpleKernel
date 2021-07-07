
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "string.h"
#include "stdio.h"
#include "common.h"
#include "firstfit.h"
#include "pmm.h"

size_t     PMM::total_pages = 0;
ALLOCATOR *PMM::allocator   = nullptr;

PMM::PMM(void) {
    return;
}

PMM::~PMM(void) {
    return;
}

bool PMM::init(void) {
    // TODO: 这三个要动态获取
    start       = COMMON::KERNEL_START_ADDR;
    length      = COMMON::PMM_SIZE;
    total_pages = length / COMMON::PAGE_SIZE;
    // 分配器使用 fitst fit 分配器
    static FIRSTFIT first_fit_allocator(start, length);
    allocator = (ALLOCATOR *)&first_fit_allocator;
    printf("pmm_init\n");
    return true;
}

const void *PMM::alloc_page(void) {
    return 0;
}

const void *PMM::alloc_pages(size_t _len) {
    (void)_len;
    return 0;
}

bool PMM::alloc_pages(void *_addr, size_t _len) {
    (void)_addr;
    (void)_len;
    return true;
}

void PMM::free_page(const void *_addr) {
    (void)_addr;
    return;
}

void PMM::free_pages(const void *_addr, size_t _len) {
    (void)_addr;
    (void)_len;
    return;
}

uint64_t PMM::get_used_pages_count(void) const {
    return 0;
}

uint64_t PMM::get_free_pages_count(void) const {
    return 0;
}

PMM pmm;

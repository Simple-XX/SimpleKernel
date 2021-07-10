
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "string.h"
#include "common.h"
#include "multiboot2.h"
#include "gdt.h"
#include "e820.h"
#include "pmm.h"

// TODO: x86 下的数据长度问题
// 将 uint64_t 改为 size_t 后可解决

const void *PMM::start       = nullptr;
size_t      PMM::length      = 0;
size_t      PMM::total_pages = 0;
ALLOCATOR * PMM::allocator   = nullptr;

PMM::PMM(void) {
    return;
}

PMM::~PMM(void) {
    return;
}

bool PMM::init(void) {
    // 因为 GDT 是 x86 遗毒，所以在这里处理
    GDT::init();
    e820map_t e820map;
    bzero(&e820map, sizeof(e820map_t));
    // TODO: 处理不能使用的内存，在虚拟内存映射时需要考虑
    MULTIBOOT2::get_e820(e820map);
    start       = COMMON::KERNEL_START_ADDR;
    length      = COMMON::PMM_SIZE;
    total_pages = length / COMMON::PAGE_SIZE;
    // 分配器使用 fitst fit 分配器
    static FIRSTFIT first_fit_allocator(start, length);
    allocator = (ALLOCATOR *)&first_fit_allocator;
    // 将内核已使用部分划分出来
    // 内核实际占用页数
    size_t kernel_pages =
        ((uint8_t *)COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         (uint8_t *)COMMON::ALIGN(COMMON::KERNEL_START_ADDR,
                                  COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    if (alloc_pages(const_cast<void *>(COMMON::KERNEL_START_ADDR),
                    kernel_pages) == true) {
        printf("pmm_init\n");
        return true;
    }
    else {
        return false;
    }
}

void *PMM::alloc_page(void) {
    return allocator->alloc(1);
}

void *PMM::alloc_pages(size_t _len) {
    return allocator->alloc(_len);
}

bool PMM::alloc_pages(void *_addr, size_t _len) {
    return allocator->alloc(_addr, _len);
}

void PMM::free_page(void *_addr) {
    allocator->free(_addr, 1);
    return;
}

void PMM::free_pages(void *_addr, size_t _len) {
    allocator->free(_addr, _len);
    return;
}

uint64_t PMM::get_used_pages_count(void) {
    return allocator->get_used_pages_count();
}

uint64_t PMM::get_free_pages_count(void) {
    return allocator->get_free_pages_count();
}

PMM pmm;

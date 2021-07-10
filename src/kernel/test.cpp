
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.cpp for Simple-XX/SimpleKernel.

#include "common.h"
#include "stdio.h"
#include "iostream"
#include "assert.h"
#include "pmm.h"
#include "kernel.h"

int32_t test_pmm(void) {
    size_t free_pages = PMM::get_free_pages_count();
    auto   kernel_pages =
        ((uint8_t *)COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         (uint8_t *)COMMON::ALIGN(COMMON::KERNEL_START_ADDR,
                                  COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    assert(free_pages == (COMMON::PMM_SIZE / COMMON::PAGE_SIZE) - kernel_pages);
    size_t used_pages = PMM::get_used_pages_count();
    assert(used_pages == kernel_pages);
    auto addr1 = PMM::alloc_pages(2);
    assert(PMM::get_used_pages_count() == 2 + kernel_pages);
    auto addr2 = PMM::alloc_pages(3);
    assert(PMM::get_used_pages_count() == 5 + kernel_pages);
    auto addr3 = PMM::alloc_pages(100);
    assert(PMM::get_used_pages_count() == 105 + kernel_pages);
    auto addr4 = PMM::alloc_pages(100);
    assert(PMM::get_used_pages_count() == 205 + kernel_pages);
    auto addr5 = PMM::alloc_pages(0xFFFFFFFF);
    assert(addr5 == nullptr);
    PMM::free_pages(addr1, 2);
    PMM::free_pages(addr2, 3);
    PMM::free_pages(addr3, 100);
    PMM::free_pages(addr4, 100);
    assert(PMM::get_free_pages_count() == free_pages);
    printf("pmm test done.\n");
    return 0;
}


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

    auto addr1 = pmm.alloc_pages(2);
    printf("addr1: 0x%p\n", addr1);
    auto addr2 = pmm.alloc_pages(3);
    printf("addr2: 0x%p\n", addr2);
    auto addr3 = pmm.alloc_pages(100);
    printf("addr3: 0x%p\n", addr3);
    auto addr4 = pmm.alloc_pages(100);
    printf("addr4: 0x%p\n", addr4);
    auto addr5 = pmm.alloc_pages(0x8000000);
    printf("addr5: 0x%p\n", addr5);

    // TODO: 分配的地址应该在相应区域内
    // uint32_t cd    = 0xCD;
    // uint8_t *addr1 = nullptr;
    // uint8_t *addr2      = nullptr;
    // uint8_t *addr3      = nullptr;
    // uint8_t *addr4      = nullptr;
    // uint32_t free_count = pmm.free_pages_count(COMMON::NORMAL);
    // addr1 = (uint8_t *)pmm.alloc_page(0x9F, COMMON::NORMAL);
    // if (addr1 != nullptr) {
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 0x9F);
    //     *(uint32_t *)addr1 = cd;
    //     assert((*(uint32_t *)addr1 == cd));
    //     pmm.free_page(addr1, 0x9F, COMMON::NORMAL);
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    // }
    // addr2 = (uint8_t *)pmm.alloc_page(1, COMMON::NORMAL);
    // if (addr2 != nullptr) {
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1);
    //     *(int *)addr2 = cd;
    //     assert((*(uint32_t *)addr2 == cd));
    //     pmm.free_page(addr2, 1, COMMON::NORMAL);
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    // }
    // addr3 = (uint8_t *)pmm.alloc_page(1024, COMMON::NORMAL);
    // if (addr3 != nullptr) {
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1024);
    //     *(int *)addr3 = cd;
    //     assert((*(uint32_t *)addr3 == cd));
    //     pmm.free_page(addr3, 1024, COMMON::NORMAL);
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    // }
    // addr4 = (uint8_t *)pmm.alloc_page(4096, COMMON::NORMAL);
    // if (addr4 != nullptr) {
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 4096);
    //     *(int *)addr4 = cd;
    //     assert((*(uint32_t *)addr4 == cd));
    //     pmm.free_page(addr4, 4096, COMMON::NORMAL);
    //     assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    // }
    // printf("pmm test done.\n");
    return 0;
}

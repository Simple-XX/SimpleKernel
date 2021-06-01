
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "common.h"
#include "stdio.h"
#include "iostream"
#include "assert.h"
#include "pmm.h"
#include "kernel.h"

#if defined(__i386__) || defined(__x86_64__)
#include "multiboot2.h"
#endif

// 内核入口
void kernel_main(uint32_t size __attribute__((unused)),
                 void *   addr __attribute__((unused))) {
#if defined(__i386__) || defined(__x86_64__)
    // __x86_64__ 时用于传递 grub 信息
    MULTIBOOT2::multiboot2_init(size, addr);
#endif
    cpp_init();
    // 物理内存初始化
    pmm.init();
    // 测试物理内存
    test_pmm();
    show_info();
    while (1) {
        ;
    }
    assert(0);
    return;
}

int32_t test_pmm(void) {
    // TODO: 分配的地址应该在相应区域内
    uint32_t cd         = 0xCD;
    uint8_t *addr1      = nullptr;
    uint8_t *addr2      = nullptr;
    uint8_t *addr3      = nullptr;
    uint8_t *addr4      = nullptr;
    uint32_t free_count = pmm.free_pages_count(COMMON::NORMAL);
    addr1               = (uint8_t *)pmm.alloc_page(0x9F, COMMON::NORMAL);
    if (addr1 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 0x9F);
        *(uint32_t *)addr1 = cd;
        assert((*(uint32_t *)addr1 == cd));
        pmm.free_page(addr1, 0x9F, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    addr2 = (uint8_t *)pmm.alloc_page(1, COMMON::NORMAL);
    if (addr2 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1);
        *(int *)addr2 = cd;
        assert((*(uint32_t *)addr2 == cd));
        pmm.free_page(addr2, 1, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    addr3 = (uint8_t *)pmm.alloc_page(1024, COMMON::NORMAL);
    if (addr3 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1024);
        *(int *)addr3 = cd;
        assert((*(uint32_t *)addr3 == cd));
        pmm.free_page(addr3, 1024, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    addr4 = (uint8_t *)pmm.alloc_page(4096, COMMON::NORMAL);
    if (addr4 != nullptr) {
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 4096);
        *(int *)addr4 = cd;
        assert((*(uint32_t *)addr4 == cd));
        pmm.free_page(addr4, 4096, COMMON::NORMAL);
        assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    }
    printf("pmm test done.\n");
    return 0;
}

void show_info(void) {
    info("kernel in memory start: 0x%X, end 0x%X\n", COMMON::KERNEL_START_ADDR,
         COMMON::KERNEL_END_ADDR);
    info("kernel in memory start4k: 0x%X, end4k 0x%X, KERNEL_SIZE:0x%X\n",
         COMMON::KERNEL_START_4K, COMMON::KERNEL_END_4K, COMMON::KERNEL_SIZE);
    info("kernel in memory size: %d KB, %d pages\n",
         ((uint8_t *)COMMON::KERNEL_END_ADDR -
          (uint8_t *)COMMON::KERNEL_START_ADDR) /
             1024,
         ((uint8_t *)COMMON::KERNEL_END_4K -
          (uint8_t *)COMMON::KERNEL_START_4K) /
             COMMON::PAGE_SIZE);
    info("Simple Kernel.\n");
    return;
}

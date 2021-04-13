
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "common.h"
#include "color.h"
#include "assert.h"
#include "kernel.h"

IO KERNEL::io;

KERNEL::KERNEL(void) : pmm(PMM()) {
    cpp_init();
    // 物理内存管理初始化
    // pmm.init();
    // 测试物理内存
    // test_pmm();
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

int32_t KERNEL::test_pmm(void) {
    // TODO: 分配的地址应该在相应区域内
    uint32_t cd         = 0xCD;
    uint8_t *addr1      = nullptr;
    uint8_t *addr2      = nullptr;
    uint8_t *addr3      = nullptr;
    uint8_t *addr4      = nullptr;
    uint32_t free_count = pmm.free_pages_count(COMMON::NORMAL);
    addr1               = (uint8_t *)pmm.alloc_page(0x9F, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 0x9F);
    *(uint32_t *)addr1 = cd;
    assert((*(uint32_t *)addr1 == cd));
    pmm.free_page(addr1, 0x9F, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    addr2 = (uint8_t *)pmm.alloc_page(1, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1);
    *(int *)addr2 = cd;
    assert((*(uint32_t *)addr2 == cd));
    pmm.free_page(addr2, 1, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    addr3 = (uint8_t *)pmm.alloc_page(1024, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 1024);
    *(int *)addr3 = cd;
    assert((*(uint32_t *)addr3 == cd));
    pmm.free_page(addr3, 1024, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    addr4 = (uint8_t *)pmm.alloc_page(4096, COMMON::NORMAL);
    assert((*(uint32_t *)addr4 == cd));
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count - 4096);
    *(int *)addr4 = cd;
    pmm.free_page(addr4, 4096, COMMON::NORMAL);
    assert(pmm.free_pages_count(COMMON::NORMAL) == free_count);
    io.printf("pmm test done.\n");
    return 0;
}

void KERNEL::show_info(void) {
    // BUG: raspi2 下不能正常输出链接脚本中的地址
    io.info("kernel in memory start: 0x%X, end 0x%X\n",
            COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR);
    io.info("kernel in memory start4k: 0x%X, end4k 0x%X, KERNEL_SIZE:0x%X\n",
            COMMON::KERNEL_START_4K, COMMON::KERNEL_END_4K,
            COMMON::KERNEL_SIZE);
    io.info("kernel in memory size: %d KB, %d pages\n",
            ((uint8_t *)COMMON::KERNEL_END_ADDR -
             (uint8_t *)COMMON::KERNEL_START_ADDR) /
                1024,
            ((uint8_t *)COMMON::KERNEL_END_4K -
             (uint8_t *)COMMON::KERNEL_START_4K) /
                COMMON::PAGE_SIZE);
    io.info("Simple Kernel.\n");
    return;
}


// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// test.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "assert.h"
#include "test.h"
#include "debug.h"
#include "pmm.h"

bool test(void) {
    test_pmm();
    // test_libc();
    return true;
}

bool test_libc(void) {
    return true;
}

// TODO: 完善测试
bool test_pmm(void) {
    ptr_t    allc_addr1   = 0;
    ptr_t    allc_addr2   = 0;
    ptr_t    allc_addr3   = 0;
    ptr_t    allc_addr4   = 0;
    uint32_t dma_free     = pmm_free_pages_count(DMA);
    uint32_t normal_free  = pmm_free_pages_count(NORMAL);
    uint32_t highmem_free = pmm_free_pages_count(HIGHMEM);
    // 单次分配&回收
    allc_addr1 = pmm_alloc(1, DMA);
    pmm_free_page(allc_addr1, 1, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free_page(allc_addr1, 1, DMA) error\n");
    allc_addr1 = pmm_alloc(9000, DMA);
    pmm_free_page(allc_addr1, 9000, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free_page(allc_addr1, 9000, DMA) error\n");

    allc_addr1 = pmm_alloc(1, NORMAL);
    pmm_free_page(allc_addr1, 1, NORMAL);
    assert(normal_free == pmm_free_pages_count(NORMAL),
           "pmm_free_page(allc_addr1, 1, NORMAL) error\n");
    allc_addr1 = pmm_alloc(9000, NORMAL);
    pmm_free_page(allc_addr1, 9000, NORMAL);
    assert(normal_free == pmm_free_pages_count(NORMAL),
           "pmm_free_page(allc_addr1, 9000, NORMAL) error\n");

    // pg
    allc_addr1 = pmm_alloc(1, HIGHMEM);
    pmm_free_page(allc_addr1, 1, HIGHMEM);
    assert(highmem_free == pmm_free_pages_count(HIGHMEM),
           "pmm_free_page(allc_addr1, 1, HIGHMEM) error\n");
    allc_addr1 = pmm_alloc(9000, HIGHMEM);
    pmm_free_page(allc_addr1, 9000, HIGHMEM);
    assert(highmem_free == pmm_free_pages_count(HIGHMEM),
           "pmm_free_page(allc_addr1, 9000, HIGHMEM) error\n");

    // 连续分配&回收
    allc_addr1 = pmm_alloc(1, DMA);
    allc_addr2 = pmm_alloc(2, DMA);
    allc_addr3 = pmm_alloc(3, DMA);
    allc_addr4 = pmm_alloc(4, DMA);
    pmm_free_page(allc_addr1, 1, DMA);
    pmm_free_page(allc_addr2, 2, DMA);
    pmm_free_page(allc_addr3, 3, DMA);
    pmm_free_page(allc_addr4, 4, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free_page(allc_addr1, 1, DMA) error\n");
    allc_addr1 = pmm_alloc(9000, DMA);
    allc_addr2 = pmm_alloc(9000, DMA);
    allc_addr3 = pmm_alloc(9000, DMA);
    allc_addr4 = pmm_alloc(9000, DMA);
    pmm_free_page(allc_addr1, 9000, DMA);
    pmm_free_page(allc_addr2, 9000, DMA);
    pmm_free_page(allc_addr3, 9000, DMA);
    pmm_free_page(allc_addr4, 9000, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free_page(allc_addr1, 9000, DMA) error\n");
    printk_debug("3\n");
    // 边界测试

    // 极限测试
    printk_info("pmm test done.\n");
    return true;
}

#ifdef __cplusplus
}
#endif

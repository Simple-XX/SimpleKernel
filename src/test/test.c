
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.c for Simple-XX/SimpleKernel.

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
    test_libc();
    test_pmm();
    test_vmm();
    return true;
}

// TODO
bool test_libc(void) {
    printk_test("libc test done.\n");
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
    pmm_free(allc_addr1, 1, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free(allc_addr1, 1, DMA) error\n");
    allc_addr1 = pmm_alloc(9000, DMA);
    pmm_free(allc_addr1, 9000, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free(allc_addr1, 9000, DMA) error\n");

    allc_addr1 = pmm_alloc(1, NORMAL);
    pmm_free(allc_addr1, 1, NORMAL);
    assert(normal_free == pmm_free_pages_count(NORMAL),
           "pmm_free(allc_addr1, 1, NORMAL) error\n");
    allc_addr1 = pmm_alloc(9000, NORMAL);
    pmm_free(allc_addr1, 9000, NORMAL);
    assert(normal_free == pmm_free_pages_count(NORMAL),
           "pmm_free(allc_addr1, 9000, NORMAL) error\n");

    allc_addr1 = pmm_alloc(1, HIGHMEM);
    pmm_free(allc_addr1, 1, HIGHMEM);
    assert(highmem_free == pmm_free_pages_count(HIGHMEM),
           "pmm_free(allc_addr1, 1, HIGHMEM) error\n");
    allc_addr1 = pmm_alloc(9000, HIGHMEM);
    pmm_free(allc_addr1, 9000, HIGHMEM);
    assert(highmem_free == pmm_free_pages_count(HIGHMEM),
           "pmm_free(allc_addr1, 9000, HIGHMEM) error\n");

    // 连续分配&回收
    allc_addr1 = pmm_alloc(1, DMA);
    allc_addr2 = pmm_alloc(2, DMA);
    allc_addr3 = pmm_alloc(3, DMA);
    allc_addr4 = pmm_alloc(4, DMA);
    pmm_free(allc_addr1, 1, DMA);
    pmm_free(allc_addr2, 2, DMA);
    pmm_free(allc_addr3, 3, DMA);
    pmm_free(allc_addr4, 4, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free(allc_addr1, 1, DMA) error\n");
    allc_addr1 = pmm_alloc(9000, DMA);
    allc_addr2 = pmm_alloc(9000, DMA);
    allc_addr3 = pmm_alloc(9000, DMA);
    allc_addr4 = pmm_alloc(9000, DMA);
    pmm_free(allc_addr1, 9000, DMA);
    pmm_free(allc_addr2, 9000, DMA);
    pmm_free(allc_addr3, 9000, DMA);
    pmm_free(allc_addr4, 9000, DMA);
    assert(dma_free == pmm_free_pages_count(DMA),
           "pmm_free(allc_addr1, 9000, DMA) error\n");

    // 边界测试
    // 0x00 地址不能访问
    int *dma_start = (void *)(DMA_START_ADDR + 0x01);
    *dma_start     = 0x233;
    assert(*dma_start == 0x233, "dma_start error!\n");
    // 减去一个指针大小
    int *dma_end = (void *)(DMA_START_ADDR + DMA_SIZE - 0x4);
    *dma_end     = 0xcd;
    assert(*dma_end == 0xcd, "dma_end error!\n");
    int *normal_start = (void *)(NORMAL_START_ADDR);
    *normal_start     = 0x233;
    assert(*normal_start == 0x233, "normal_start error!\n");
    int *normal_end = (void *)(NORMAL_START_ADDR + NORMAL_SIZE - 0x4);
    *normal_end     = 0xcd;
    assert(*normal_end == 0xcd, "normal_end error!\n");
    int *highmem_start = (void *)(HIGHMEM_START_ADDR);
    *highmem_start     = 0x233;
    assert(*highmem_start == 0x233, "highmem_start error!\n");
    int *highmem_end = (void *)(HIGHMEM_START_ADDR + HIGHMEM_SIZE - 0x4);
    *highmem_end     = 0xcd;
    assert(*highmem_end == 0xcd, "highmem_end error!\n");

    // 极限测试

    printk_test("pmm test done.\n");
    return true;
}

// TODO
bool test_vmm(void) {
    printk_test("vmm test done.\n");
    return true;
}

#ifdef __cplusplus
}
#endif

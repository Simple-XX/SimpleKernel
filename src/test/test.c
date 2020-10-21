
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// test.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "debug.h"
#include "pmm.h"
#include "clock.h"
#include "keyboard.h"
#include "test.h"
#include "firstfit.h"

bool test_pmm(void) {
    ptr_t allc_addr = 0;
    printk_test("Test Physical Memory Alloc :\n");
    allc_addr = pmm_alloc(1, DMA);
    printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
    allc_addr = pmm_alloc(9000, DMA);
    printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
    printk_test("Free pages count: %d\n", pmm_free_pages_count(DMA));
    pmm_free_page(allc_addr, 9000, DMA);
    printk_test("Free!\n");
    printk_test("Free pages count: %d\n", pmm_free_pages_count(DMA));
    allc_addr = pmm_alloc(1, DMA);
    printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
    printk_test("Free pages count: %d\n", pmm_free_pages_count(DMA));
    allc_addr = pmm_alloc(1, DMA);
    printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
    printk_test("Free pages count: %d\n", pmm_free_pages_count(DMA));
    return true;
}

bool test_libc(void) {
    return true;
}

bool test(void) {
    test_pmm();
    // test_libc();
    return true;
}

#ifdef __cplusplus
}
#endif

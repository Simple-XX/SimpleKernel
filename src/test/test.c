
// This file is a part of MRNIU/SimpleKernel
// (https://github.com/MRNIU/SimpleKernel).
//
// test.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "debug.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "../drv/clock/include/clock.h"
#include "../drv/keyboard/include/keyboard.h"
#include "heap/heap.h"
#include "task/task.h"
#include "sched/sched.h"
#include "include/test.h"
#include "mem/firstfit.h"
#include "mem/swap.h"
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
    //	printk_test("Free pages count: %d\n", pmm_free_pages_count(DMA) );
    allc_addr = pmm_alloc(1, DMA);
    printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
    printk_test("Free pages count: %d\n", pmm_free_pages_count(DMA));
    return true;
}

bool test_vmm(void) {
    return true;
}

bool test_libc(void) {
    return true;
}

static int32_t thread2(void *arg __UNUSED__) {
    for (int i = 0; i < 100; i++) {
        printk("2");
    }
    return 2;
}

static int32_t thread3(void *arg __UNUSED__) {
    for (int i = 0; i < 100; i++) {
        printk("3");
    }
    return 3;
}

static int32_t thread4(void *arg __UNUSED__) {
    for (int i = 0; i < 100; i++) {
        printk("4");
    }
    return 4;
}

bool test_task(void) {
    pid_t pid1 = kernel_thread(thread2, NULL, 0);
    pid_t pid2 = kernel_thread(thread3, NULL, 0);
    pid_t pid3 = kernel_thread(thread4, NULL, 0);

    set_task_name(pid1, "test1");
    set_task_name(pid2, "test2");
    set_task_name(pid3, "test3");

    show_task(TASK_MAX + 1);
    show_task_mem(0);
    show_task_pt_regs(0);
    show_task_context(0);
    show_task(pid1);

    return true;
}

bool test_sched(void) {
    return true;
}

bool test_heap(void) {
    printk_test("Test Heap kmalloc :\n");
    ptr_t allc_addr1 = (ptr_t)kmalloc(1);
    printk_test("kmalloc heap addr: 0x%08X\n", allc_addr1);
    ptr_t allc_addr2 = (ptr_t)kmalloc(9000);
    printk_test("kmalloc heap addr: 0x%08X\n", allc_addr2);
    ptr_t allc_addr3 = (ptr_t)kmalloc(4095);
    printk_test("kmalloc heap addr: 0x%08X\n", allc_addr3);
    ptr_t allc_addr4 = (ptr_t)kmalloc(12);
    printk_test("kmalloc heap addr: 0x%08X\n", allc_addr4);
    printk_test("Test Heap kfree: 0x%08X\n", allc_addr1);
    kfree((ptr_t)allc_addr1);
    printk_test("Test Heap kfree: 0x%08X\n", allc_addr2);
    kfree((ptr_t)allc_addr2);
    printk_test("Test Heap kfree: 0x%08X\n", allc_addr3);
    kfree((ptr_t)allc_addr3);
    printk_test("Test Heap kfree: 0x%08X\n", allc_addr4);
    kfree((ptr_t)allc_addr4);
    ptr_t new_addr = (ptr_t)kmalloc(9000);
    printk_test("New kmalloc heap addr: 0x%08X\n", new_addr);
    return true;
}

bool test(void) {
    test_pmm();
    // test_vmm();
    // test_libc();
    // test_heap();
    // test_task();
    // test_sched();
    return true;
}

#ifdef __cplusplus
}
#endif

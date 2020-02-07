
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
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

bool test_pmm(void) {
	ptr_t allc_addr = 0;
	printk_test("Test pmm_alloc :\n");
	allc_addr = pmm_alloc(1);
	printk_test("pmm_alloc addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc(9000);
	printk_test("pmm_alloc addr: 0x%08X\n", allc_addr);
	printk_test("Free pages count: %d\n", pmm_free_pages_count() );
	pmm_free(allc_addr, 9000);
	printk_test("Free pages count: %d\n", pmm_free_pages_count() );
	allc_addr = pmm_alloc(1);
	printk_test("pmm_alloc addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc(1);
	printk_test("pmm_alloc addr: 0x%08X\n", allc_addr);
	printk_test("Free pages count: %d\n", pmm_free_pages_count() );
	return true;
}

bool test_vmm() {
	return true;
}

bool test_libc() {
	return true;
}

bool test_task() {
	return true;
}

bool test_sched() {
	return true;
}

bool test_heap() {
	printk_test("Test Heap kmalloc :\n");
	ptr_t allc_addr1 = (ptr_t)kmalloc(1);
	printk_test("kmalloc heap addr: 0x%08X\n", allc_addr1);
	ptr_t allc_addr2 = (ptr_t)kmalloc(9000);
	printk_test("kmalloc heap addr: 0x%08X\n", allc_addr2);
	ptr_t allc_addr3 = (ptr_t)kmalloc(4095);
	printk_test("kmalloc heap addr: 0x%08X\n", allc_addr3);
	ptr_t allc_addr4 = (ptr_t)kmalloc(12);
	printk_test("kmalloc heap addr: 0x%08X\n", allc_addr4);
	printk_test("Test Heap kfree :\n");
	kfree( (ptr_t)allc_addr1);
	kfree( (ptr_t)allc_addr2);
	kfree( (ptr_t)allc_addr3);
	kfree( (ptr_t)allc_addr4);
	ptr_t new_addr = (ptr_t)kmalloc(9000);
	printk_test("New kmalloc heap addr: 0x%08X\n", new_addr);
	return true;
}

bool test(void) {
	test_pmm();
	test_vmm();
	test_libc();
	test_heap();
	// test_task();
	// test_sched();
	return true;
}

#ifdef __cplusplus
}
#endif

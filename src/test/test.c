
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// test.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/test.h"

bool test_pmm(void) {
	ptr_t allc_addr = 0;
	printk_test("Test Physical Memory Alloc :\n");
	allc_addr = pmm_alloc(1);
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc(9000);
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	printk_test("Free pages count: %d\n", pmm_free_pages_count() );
	pmm_free_page(allc_addr, 9000);
	printk_test("Free!\n");
	printk_test("Free pages count: %d\n", pmm_free_pages_count() );
	allc_addr = pmm_alloc(1);
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc(1);
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	printk_test("Free pages count: %d\n", pmm_free_pages_count() );
	return true;
}

bool test_vmm() {
	return true;
}

bool test_libc() {
	return true;
}

bool test(void) {
	test_pmm();
	test_vmm();
	// test_libc();
	return true;
}

#ifdef __cplusplus
}
#endif

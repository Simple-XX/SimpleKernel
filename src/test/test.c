
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// test.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/test.h"

bool test_pmm() {
	ptr_t allc_addr = 0;
	printk_test("Test Physical Memory Alloc :\n");
	allc_addr = pmm_alloc_page();
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc_page();
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc_page();
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
	allc_addr = pmm_alloc_page();
	printk_test("Alloc Physical Addr: 0x%08X\n", allc_addr);
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
	return true;
}

#ifdef __cplusplus
}
#endif

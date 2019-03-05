
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// test.c for MRNIU/SimpleKernel.

#include "test.h"

bool test_pmm(){
		uint32_t allc_addr = NULL;
		printk_color(COL_TEST, "[TEST] ");
		printk_color(red, "Test Physical Memory Alloc :\n");
		allc_addr = pmm_alloc_page();
		printk_color(COL_TEST, "[TEST] ");
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		allc_addr = pmm_alloc_page();
		printk_color(COL_TEST, "[TEST] ");
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		allc_addr = pmm_alloc_page();
		printk_color(COL_TEST, "[TEST] ");
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		allc_addr = pmm_alloc_page();
		printk_color(COL_TEST, "[TEST] ");
		printk_color(red, "Alloc Physical Addr: 0x%08X\n", allc_addr);
		return true;
}

bool test(void){
		test_pmm();
		return true;
}

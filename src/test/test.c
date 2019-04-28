
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// test.c for MRNIU/SimpleKernel.

#include "test.h"

bool test_pmm(){
		uint64_t allc_addr = 0;
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

bool test_vmm(){
		return true;
}

bool test_libc(){
		return true;
}


void test_heap()
{
		printk_color(magenta, "Test kmalloc() && kfree() now ...\n\n");
		void *addr1 = kmalloc(50);
		printk("kmalloc    50 byte in 0x%X\n", addr1);
		void *addr2 = kmalloc(500);
		printk("kmalloc   500 byte in 0x%X\n", addr2);
		void *addr3 = kmalloc(5000);
		printk("kmalloc  5000 byte in 0x%X\n", addr3);
		void *addr4 = kmalloc(50000);
		printk("kmalloc 50000 byte in 0x%X\n\n", addr4);

		printk("free mem in 0x%X\n", addr1);
		kfree(addr1);
		printk("free mem in 0x%X\n", addr2);
		kfree(addr2);
		printk("free mem in 0x%X\n", addr3);
		kfree(addr3);
		printk("free mem in 0x%X\n\n", addr4);
		kfree(addr4);
}


bool test(void){
		test_pmm();
		// test_heap();
		return true;
}

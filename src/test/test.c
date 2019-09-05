
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// test.c for MRNIU/SimpleKernel.

#include "test.h"

bool test_pmm(){
    uint32_t allc_addr = 0;
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

    uint32_t * testinfo;
    uint32_t * test222;
    uint32_t addr1 = 0;
    uint32_t addr2 = 0;

    testinfo = pmm_alloc_pages(5);
    addr1 = &testinfo;
    test222 = pmm_alloc_pages(10);
    addr2 = &test222;
    printk("addr1 = 0x%08X\n", addr1);
    printk("addr2 = 0x%08X\n", addr2);
    for(int i = 0; i < 10; i++) {
        testinfo[i] = i;
        test222[i] = i * 10;
    }

    for(int i = 0; i < 10; i++) {
        printk("testinfo %d = %d\n", i, testinfo[i]);
        printk("test222 %d = %d\n", i, test222[i]);
    }

    return true;
}

bool test_vmm(){
    return true;
}

bool test_libc(){
    return true;
}

bool test(void){
    test_pmm();
    return true;
}

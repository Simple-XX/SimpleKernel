
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "assert.h"
#include "test.h"
#include "debug.h"
#include "pmm.h"

bool test(void) {
    test_libc();
    test_pmm();
    return true;
}

// TODO
bool test_libc(void) {
    printk_test("libc test done.\n");
    return true;
}

// TODO: 完善测试
bool test_pmm(void) {
    uint32_t cd         = 0xCD;
    ptr_t    addr1      = 0x233;
    ptr_t    addr2      = 0x00;
    ptr_t    addr3      = 0x00;
    ptr_t    addr4      = 0x00;
    uint32_t free_count = pmm_free_pages_count();
    addr1               = pmm_alloc_page(0x9F);
    assert(pmm_free_pages_count() == free_count - 0x9F,
           "pmm test addr1 alloc.\n");
    *(uint32_t *)addr1 = cd;
    assert((*(uint32_t *)addr1 == cd), "pmm test addr1 assignment.\n");
    pmm_free_page(addr1, 0x9F);
    assert(pmm_free_pages_count() == free_count, "pmm test addr1 free.\n");
    addr2 = pmm_alloc_page(1);
    assert(pmm_free_pages_count() == free_count - 1, "pmm test addr2 alloc.\n");
    *(int *)addr2 = cd;
    assert((*(uint32_t *)addr2 == cd), "pmm test addr2 assignment.\n");
    pmm_free_page(addr2, 1);
    assert(pmm_free_pages_count() == free_count, "pmm test addr2 free.\n");
    addr3 = pmm_alloc_page(1024);
    assert(pmm_free_pages_count() == free_count - 1024,
           "pmm test addr3 alloc.\n");
    *(int *)addr3 = cd;
    assert((*(uint32_t *)addr3 == cd), "pmm test addr3 assignment.\n");
    pmm_free_page(addr3, 1024);
    assert(pmm_free_pages_count() == free_count, "pmm test addr3 free.\n");
    addr4 = pmm_alloc_page(4096);
    assert((*(uint32_t *)addr4 == cd), "pmm test addr4 assignment.\n");
    assert(pmm_free_pages_count() == free_count - 4096,
           "pmm test addr4 alloc.\n");
    *(int *)addr4 = cd;
    pmm_free_page(addr4, 4096);
    assert(pmm_free_pages_count() == free_count, "pmm test addr4 free.\n");

    printk_test("pmm test done.\n");
    return true;
}

#ifdef __cplusplus
}
#endif

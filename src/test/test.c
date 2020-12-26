
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
    ptr_t    addr1      = 0x233;
    ptr_t    addr2      = 0x00;
    ptr_t    addr3      = 0x00;
    ptr_t    addr4      = 0x00;
    uint32_t free_count = pmm_free_pages_count();
    addr1               = pmm_alloc_page(0x9f);
    assert(pmm_free_pages_count() == free_count - 0x9f,
           "pmm test addr1 alloc.\n");
    pmm_free_page(addr1, 0x9f);
    assert(pmm_free_pages_count() == free_count, "pmm test addr1 free.\n");
    addr2 = pmm_alloc_page(1);
    assert(pmm_free_pages_count() == free_count - 1, "pmm test addr2 alloc.\n");
    pmm_free_page(addr2, 1);
    assert(pmm_free_pages_count() == free_count, "pmm test addr2 free.\n");
    addr3 = pmm_alloc_page(1024);
    assert(pmm_free_pages_count() == free_count - 1024,
           "pmm test addr3 alloc.\n");
    pmm_free_page(addr3, 1024);
    assert(pmm_free_pages_count() == free_count, "pmm test addr3 free.\n");
    addr4 = pmm_alloc_page(4096);
    assert(pmm_free_pages_count() == free_count - 4096,
           "pmm test addr4 alloc.\n");
    pmm_free_page(addr4, 4096);
    assert(pmm_free_pages_count() == free_count, "pmm test addr4 free.\n");

    printk_test("pmm test done.\n");
    return true;
}

#ifdef __cplusplus
}
#endif

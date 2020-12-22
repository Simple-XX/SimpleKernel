
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
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
    printk_test("pmm test done.\n");
    return true;
}

#ifdef __cplusplus
}
#endif


// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// test.h for MRNIU/SimpleKernel.

#ifndef _TEST_H_
#define _TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "debug.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "../drv/clock/include/clock.h"
#include "../drv/keyboard/include/keyboard.h"

bool test(void);
bool test_pmm(void);
bool test_vmm(void);
bool test_libc(void);
bool test_heap(void);

#ifdef __cplusplus
}
#endif

#endif /* _TEST_H_ */

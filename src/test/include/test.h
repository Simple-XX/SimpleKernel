
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// test.h for MRNIU/SimpleKernel.

#ifndef _TEST_H_
#define _TEST_H_

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "../kernel/include/multiboot2.h"
#include "../kernel/include/debug.h"
#include "mm/include/pmm.h"
#include "mm/include/vmm.h"
#include "../drv/clock/include/clock.h"
#include "../drv/keyboard/include/keyboard.h"
#include "../kernel/include/heap.h"

bool test(void);
bool test_pmm(void);
bool test_vmm(void);
bool test_libc(void);

#endif

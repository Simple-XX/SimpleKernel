
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// test.h for MRNIU/SimpleKernel.

#ifndef _TEST_H_
#define _TEST_H_

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "multiboot2.h"
#include "debug.h"
#include "mm/pmm.h"
#include "mm/mm.h"
#include "intr/clock.h"
#include "drv/keyboard.h"

bool test(void);
bool test_pmm(void);
bool test_vmm(void);
bool test_libc(void);

#endif


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// test.h for Simple-XX/SimpleKernel.

#ifndef _TEST_H_
#define _TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

bool test(void);

// C 库
bool test_libc(void);

// 物理内存
bool test_pmm(void);

// 虚拟内存
bool test_vmm(void);

#ifdef __cplusplus
}
#endif

#endif /* _TEST_H_ */

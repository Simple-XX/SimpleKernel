
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// test.h for SimpleXX/SimpleKernel.

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

#ifdef __cplusplus
}
#endif

#endif /* _TEST_H_ */

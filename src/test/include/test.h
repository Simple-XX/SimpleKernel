
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
bool test_pmm(void);
bool test_vmm(void);
bool test_libc(void);
bool test_heap(void);
bool test_task(void);
bool test_sched(void);

#ifdef __cplusplus
}
#endif

#endif /* _TEST_H_ */

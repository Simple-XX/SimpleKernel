
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// assert.h for SimpleXX/SimpleKernel.

#ifndef _ASSERT_H_
#define _ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#define assert(test, info)                                                     \
    if (!(test)) {                                                             \
        printk_err(info);                                                      \
        __asm__ volatile("cli" ::: "memory");                                  \
        while (1) {};                                                          \
    }

#ifdef __cplusplus
}
#endif

#endif /* _ASSERT_H_ */

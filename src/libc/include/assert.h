
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// assert.h for Simple-XX/SimpleKernel.

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

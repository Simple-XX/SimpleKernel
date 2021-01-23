
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// assert.h for Simple-XX/SimpleKernel.

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include "io.h"

#define assert(test, info)                                                     \
    if (!(test)) {                                                             \
        io.printf(info);                                                       \
        asm("mov $0xCDCD, %eax");                                              \
        __asm__ volatile("cli" ::: "memory");                                  \
        while (1) {};                                                          \
    }

#endif /* _ASSERT_H_ */

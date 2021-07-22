
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

#define assert(e) ((void)((e) ? ((void)0) : __assert(#e, __FILE__, __LINE__)))
#define __assert(e, file, line)                                                \
    ((void)err("%s:%d: failed assertion `%s'\n", file, line, e))

#ifdef __cplusplus
}
#endif

#endif /* _ASSERT_H_ */

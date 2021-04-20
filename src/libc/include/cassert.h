
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// cassert.h for Simple-XX/SimpleKernel.

#ifndef _CASSERT_H_
#define _CASSERT_H_

#include "stdio.h"

#define assert(e) ((void)((e) ? ((void)0) : __assert(#e, __FILE__, __LINE__)))
#define __assert(e, file, line)                                                \
    ((void)err("%s:%d: failed assertion `%s'\n", file, line, e))

#endif /* _CASSERT_H_ */

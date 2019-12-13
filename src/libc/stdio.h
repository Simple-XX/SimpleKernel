
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// stdio.h for MRNIU/SimpleKernel.

#ifndef _STDIO_H_
#define _STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

int32_t printk(const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H_ */

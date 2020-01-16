
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// assert.h for MRNIU/SimpleKernel.

#ifndef _ASSERT_H_
#define _ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#define assert(test, info) \
		if(!(test)) { \
				printk_err(info); \
				while(1); \
		}

#ifdef __cplusplus
}
#endif

#endif /* _ASSERT_H_ */

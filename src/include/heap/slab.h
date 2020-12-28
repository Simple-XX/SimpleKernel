
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// slab.h for Simple-XX/SimpleKernel.

#ifndef _SLAB_H_
#define _SLAB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "heap.h"

// 用于管理堆物理地址
extern heap_manage_t slab_manage;

#ifdef __cplusplus
}
#endif

#endif /* _SLAB_H_ */


// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// slab.h for SimpleXX/SimpleKernel.

#ifndef _SLAB_H_
#define _SLAB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "heap/heap.h"
#include "mem/swap.h"
// 用于管理堆物理地址
extern heap_manage_t slab_manage;
#ifdef __cplusplus
}
#endif

#endif /* _SLAB_H_ */

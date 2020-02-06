
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// slab.h for MRNIU/SimpleKernel.

#ifndef _SLAB_H_
#define _SLAB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "heap/heap.h"

// 用于管理物理地址
extern heap_manage_t slab_manage;

#ifdef __cplusplus
}
#endif

#endif /* _SLAB_H_ */

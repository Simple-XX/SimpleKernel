
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// heap.h for MRNIU/SimpleKernel.

#ifndef _HEAP_H_
#define _HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"

// 堆起始地址 2GB 处，向上增长
#define HEAP_START      (0x80000000)
// 堆最大容量 4MB
#define HEAP_MAX_SIZE   (0x400000)

// 内存块管理结构
typedef
        struct heap_header {
	struct heap_header * prev;    // 前后内存块管理结构指针
	struct heap_header * next;
	uint32_t allocated : 1; // 该内存块是否已经被申请
	uint32_t length : 31;   // 当前内存块的长度
} heap_header_t;

// 初始化堆
void heap_init(void);

// 内存申请
ptr_t kmalloc(size_t len);

// 内存释放
void kfree(ptr_t p);

#ifdef __cplusplus
}
#endif

#endif /* _HEAP_H_ */

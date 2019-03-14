
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// heap.c for MRNIU/SimpleKernel.

#include "heap.h"


// // 申请内存块
// static void alloc_chunk(uint32_t start, uint32_t len);
//
// // 释放内存块
// static void free_chunk(header_t *chunk);
//
// // 切分内存块
// static void split_chunk(header_t *chunk, uint32_t len);
//
// // 合并内存块
// static void glue_chunk(header_t *chunk);
//
// static uint32_t heap_max = HEAP_START;
//
// // 内存块管理头指针
// static header_t *heap_first;

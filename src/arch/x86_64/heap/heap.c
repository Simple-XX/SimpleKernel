
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// heap.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "cpu.hpp"
#include "mem/slab.h"
#include "heap/heap.h"

static const heap_manage_t * heap_manager = &slab_manage;

// 初始化堆
void heap_init(void) {
	cpu_cli();
	heap_manager->heap_manage_init(HEAP_START);
	printk_info("heap_init\n");
	return;
}

// 内存申请
ptr_t kmalloc(size_t len) {
	ptr_t addr = 0;
	addr = heap_manager->heap_manage_malloc(len);
	return addr;
}

// 内存释放
void kfree(ptr_t p) {
	heap_manager->heap_manage_free(p);
	return;
}

#ifdef __cplusplus
}
#endif

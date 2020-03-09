
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
ptr_t kmalloc(size_t byte) {
	ptr_t addr = 0;
	addr = heap_manager->heap_manage_malloc_align(byte, 0);
	return addr;
}

// 内存申请，单位为 Byte，align 为对齐大小
ptr_t kmalloc_align(size_t byte, size_t align) {
	ptr_t addr = 0;
	addr = heap_manager->heap_manage_malloc_align(byte, align);
	return addr;
}

// 内存释放
void kfree(ptr_t addr) {
	heap_manager->heap_manage_free(addr);
	return;
}

#ifdef __cplusplus
}
#endif

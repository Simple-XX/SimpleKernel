
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// heap.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "heap/heap.h"
#include "mem/pmm.h"
#include "mem/vmm.h"

// 空闲块链表
static heap_header_t * heap_free_chunk_ll = NULL;
// 当前堆顶
static ptr_t heap_top = HEAP_START;

// 初始化堆
void heap_init(void) {
	// 首先给链表中添加一个大小为 1 页的块
	// 物理地址
	ptr_t pa = pmm_alloc(1);
	printk_debug("pa = %X\n", pa);
	// 虚拟地址
	ptr_t va = (ptr_t)HEAP_START;
	printk_debug("va = %X\n", va);
	map(pgd_kernel, va, pa, VMM_PAGE_PRESENT | VMM_PAGE_RW);
	heap_free_chunk_ll = (heap_header_t *)va;
	heap_free_chunk_ll->allocated = 0;
	heap_free_chunk_ll->length = VMM_PAGE_SIZE - sizeof(heap_header_t);
	heap_top += VMM_PAGE_SIZE;
}

// 内存申请
ptr_t kmalloc(size_t len) {
	// 需要的空间 = 申请的空间大小 + 保存信息用的空间大小
	len += sizeof(heap_header_t);
	heap_header_t * chunk = (heap_header_t *)HEAP_START;
	heap_header_t * prev = NULL;

	// 遍历寻找未分配且空间足够的块
	while(chunk) {
		if(chunk->allocated == 0 && chunk->length > len) {
			return (ptr_t)(chunk + (sizeof(heap_header_t) ) );
		}
		else {
			prev = chunk;
			chunk = chunk->next;
		}
	}
	// 没找到能用的，map 更多的内存
	ptr_t pa = pmm_alloc(1);
	// 虚拟地址
	ptr_t va = (ptr_t)heap_top;
	map(pgd_kernel, va, pa, VMM_PAGE_PRESENT | VMM_PAGE_RW);
	chunk = (heap_header_t *)va;
	chunk->allocated = 1;
	chunk->length = VMM_PAGE_SIZE - sizeof(heap_header_t);
	prev->next = chunk;
	chunk->prev = prev;
	chunk->next = NULL;
	heap_top += VMM_PAGE_SIZE;
	return (ptr_t)(chunk + (sizeof(heap_header_t) ) );
}

// 内存释放
void kfree(ptr_t p) {
	// 指针回退到管理结构，并将已使用标记置 0
	heap_header_t * header = (heap_header_t *)(p - sizeof(heap_header_t) );
	header->allocated = 0;
	// 遍历寻找未使用的块
	return;
}

#ifdef __cplusplus
}
#endif

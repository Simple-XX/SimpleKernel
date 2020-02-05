
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
	printk_debug("pa1 = %X\n", pa);
	// 虚拟地址
	ptr_t va = (ptr_t)HEAP_START;
	// printk_debug("va1 = %X\n", va);
	map(pgd_kernel, va, pa, VMM_PAGE_PRESENT | VMM_PAGE_RW);
	// printk_debug("va2 = %X\n", va);
	// printk_debug("pa2 = %X\n", pa);
	heap_free_chunk_ll = (heap_header_t *)va;
	// printk_debug("heap_free_chunk_ll = %X\n", *heap_free_chunk_ll);
	heap_free_chunk_ll->allocated = 0;
	// printk_debug("heap_free_chunk_ll->allocated = %X\n", heap_free_chunk_ll->allocated);
	heap_free_chunk_ll->length = VMM_PAGE_SIZE - sizeof(heap_header_t);
	heap_free_chunk_ll->prev = NULL;
	heap_free_chunk_ll->next = NULL;
	// printk_debug("heap_free_chunk_ll->length = %X\n", heap_free_chunk_ll->length);
	// printk_debug("heap_top1 = %X\n", heap_top);
	heap_top += VMM_PAGE_SIZE;
	// printk_debug("heap_top2 = %X\n", heap_top);
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
			chunk->allocated = 1;
			return (ptr_t)(chunk + (sizeof(heap_header_t) ) );
		}
		else {
			prev = chunk;
			chunk = chunk->next;
		}
	}
	// 没找到能用的，map 更多的内存
	ptr_t pa = pmm_alloc(1);
	// printk_debug("pa3 = %X\n", pa);
	// 虚拟地址
	ptr_t va = (ptr_t)heap_top;
	// printk_debug("va3 = %X\n", va);
	map(pgd_kernel, va, pa, VMM_PAGE_PRESENT | VMM_PAGE_RW);
	chunk = (heap_header_t *)va;
	// printk_debug("chunk = %X\n", chunk);
	chunk->allocated = 1;
	chunk->length = VMM_PAGE_SIZE - sizeof(heap_header_t);
	chunk->prev = prev;
	chunk->next = NULL;
	prev->next = chunk;
	heap_top += VMM_PAGE_SIZE;
	// printk_debug("heap_top3 = %X\n", heap_top);
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

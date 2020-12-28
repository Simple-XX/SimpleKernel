
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// heap.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "cpu.hpp"
#include "slab.h"
#include "heap.h"

static const heap_manage_t *heap_manager = &slab_manage;

// 初始化堆
void heap_init(void) {
    heap_manager->heap_manage_init((void *)HEAP_START);
    printk_info("heap_init\n");
    return;
}

// 内存申请
void *kmalloc(size_t byte) {
    void *addr = NULL;
    addr       = heap_manager->heap_manage_malloc(byte);
    return addr;
}

// 内存释放
void kfree(void *addr) {
    heap_manager->heap_manage_free(addr);
    return;
}

uint32_t heap_get_pages(void) {
    uint32_t pages = 0;
    pages          = heap_manager->heap_manage_get_pages();
    return pages;
}

uint32_t heap_get_free_bytes(void) {
    uint32_t bytes = 0;
    bytes          = heap_manager->heap_manage_get_free_bytes();
    return bytes;
}

#ifdef __cplusplus
}
#endif

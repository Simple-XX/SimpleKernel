
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

    heap_manager->heap_manage_init((ptr_t)HEAP_START);
    printk_info("heap_init\n");
    return;
}

// 内存申请
ptr_t kmalloc(size_t byte) {
    ptr_t addr = (ptr_t)NULL;
    addr       = heap_manager->heap_manage_malloc(byte);
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

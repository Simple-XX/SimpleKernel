
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// heap.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "cpu.hpp"
#include "sync.hpp"
#include "mem/slab.h"
#include "heap/heap.h"

static const heap_manage_t *heap_manager = &slab_manage;

// 初始化堆
void heap_init(void) {
    bool intr_flag = false;
    local_intr_store(intr_flag);
    {
        heap_manager->heap_manage_init(HEAP_START);
        printk_info("heap_init\n");
    }
    local_intr_restore(intr_flag);
    return;
}

// 内存申请
ptr_t kmalloc(size_t byte) {
    ptr_t addr      = (ptr_t)NULL;
    bool  intr_flag = false;
    local_intr_store(intr_flag);
    { addr = heap_manager->heap_manage_malloc(byte); }
    local_intr_restore(intr_flag);
    return addr;
}

// 内存释放
void kfree(ptr_t addr) {
    bool intr_flag = false;
    local_intr_store(intr_flag);
    { heap_manager->heap_manage_free(addr); }
    local_intr_restore(intr_flag);
    return;
}

#ifdef __cplusplus
}
#endif

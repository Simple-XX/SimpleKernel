
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// kernel.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.hpp"
#include "debug.h"
#include "assert.h"
#include "sync.hpp"
#include "include/kernel.h"
#include "clock.h"
#include "keyboard.h"
#include "test.h"
#include "linkedlist.h"

// 内核入口
void kernel_main(ptr_t magic, ptr_t addr) {
    bool intr_flag = false;
    local_intr_store(intr_flag);
    {
        // 控制台初始化
        console_init();
        printk_debug("%p, %p", magic, addr);
        // 从 multiboot 获得系统初始信息
        multiboot2_init(magic, addr);
        // GDT、IDT 初始化
        arch_init();
        // 时钟初始化
        clock_init();
        // 键盘初始化
        keyboard_init();
        // 调试模块初始化
        debug_init(magic, addr);
        // 物理内存初始化
        pmm_init();
        //测试缓冲区
        test_buffer();
        // 虚拟内存初始化
        vmm_init();
        test_swap();
        // 堆初始化
        heap_init();

        // showinfo();
        test();
        while (1)
            ;
    }
    local_intr_restore(intr_flag);

    // 永远不会执行到这里
    assert(0, "Never to be seen.\n");
    return;
}

#ifdef __cplusplus
}
#endif

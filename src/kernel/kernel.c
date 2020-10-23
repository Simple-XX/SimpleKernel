
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.hpp"
#include "debug.h"
#include "assert.h"
#include "kernel.h"
#include "clock.h"
#include "keyboard.h"
#include "buffer.h"
#include "test.h"
#include "buffer.h"

// 内核入口
void kernel_main(ptr_t magic, ptr_t addr) {
    // 控制台初始化
    console_init();
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

    // showinfo();
    // test();

    cpu_sti();
    while (1) {
        ;
    }

    // 永远不会执行到这里
    assert(0, "Never to be seen.\n");
    return;
}

#ifdef __cplusplus
}
#endif

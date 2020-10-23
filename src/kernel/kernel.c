
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel.h"
#include "cpu.hpp"

// 内核入口
void kernel_main() {
    // debug 初始化
    debug_init();
    // 控制台初始化
    console_init();
    // GDT 初始化
    gdt_init();
    // IDT 初始化
    intr_init();
    // 时钟初始化
    clock_init();
    // 键盘初始化
    keyboard_init();

    showinfo();
    cpu_sti();

    while (1) {
        ;
    }

    return;
}

#ifdef __cplusplus
}
#endif

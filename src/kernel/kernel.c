
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "assert.h"
#include "kernel.h"
#include "cpu.hpp"
#include "debug.h"
#include "clock.h"
#include "console.h"
#include "multiboot2.h"
#include "arch_init.h"
#include "keyboard.h"
#include "vmm.h"
#include "test.h"

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
    // 虚拟内存初始化
    vmm_init();
    printk_info("_______________\n");
    test();
    // showinfo();

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

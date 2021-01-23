
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "debug.h"
#include "assert.h"
#include "kernel.h"
#include "test.h"

// // 内核入口
// void kernel_main(addr_t magic, addr_t addr) {
//     // 控制台初始化
//     console_init();
//     // 从 multiboot 获得系统初始信息
//     multiboot2_init(magic, addr);
//     // GDT、IDT 初始化
//     arch_init();
//     // 时钟初始化
//     clock_init();
//     // 键盘初始化
//     keyboard_init();
//     // 调试模块初始化
//     debug_init(magic, addr);
//     // 物理内存初始化
//     pmm_init();

//     test();
//     showinfo();

//     cpu_sti();
//     while (1) {
//         ;
//     }

//     // 永远不会执行到这里
//     assert(0, "Never to be seen.\n");
//     return;
// }

void kernel_main(addr_t magic, addr_t addr) {
    KERNEL kernel(magic, addr);
    kernel.init();

#if defined(RASPI2)
    uart_init();
    char  c = '!';
    char *s = "gg";
    framebuffer_init();
    log_info("__%c__%s\n", c, s);
    // TODO: width 和 height 的值会出错
    for (uint32_t i = 0; i < FRAMEBUFFER_HEIGHT; i++) {
        for (uint32_t j = 0; j < FRAMEBUFFER_WIDTH; j++) {
            framebuffer_set_pixel(j, i, 0xAA00FF);
        }
    }
    log_info("Simple Kernel.\n");
#endif
    cpu_sti();
    while (1) {
        ;
    }
    return;
}

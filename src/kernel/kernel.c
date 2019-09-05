
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"
#include "../test/test.h"

#define DEBUG 1

// 内核入口
// 指针是 32 位的
void kernel_main(uint32_t magic, uint32_t addr) {
    console_init();     // 控制台初始化
    gdt_init();      // GDT 初始化
    idt_init();     // IDT 初始化
    clock_init();     // 时钟初始化
    keyboard_init();     // 键盘初始化
    multiboot2_init(magic, addr); // 包括 pmm_init 与 vmm_init
    // debug_init(magic, addr);
    showinfo();
    test();


    while(1);

    return;
}


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// debug.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "intr.h"
#include "cpu.hpp"
#include "debug.h"

void debug_init(addr_t magic __UNUSED__, addr_t addr __UNUSED__) {
    printk_debug("debug_init\n");
    return;
}

void print_cur_status() {
    static uint32_t round = 0;
    uint16_t        reg1, reg2, reg3, reg4;
    asm volatile("mov %%cs, %0;"
                 "mov %%ds, %1;"
                 "mov %%es, %2;"
                 "mov %%ss, %3;"
                 : "=m"(reg1), "=m"(reg2), "=m"(reg3), "=m"(reg4));

    // 打印当前的运行级别
    printk_debug("%d:  @ring %d\n", round, reg1 & 0x3);
    printk_debug("%d:  cs = %X\n", round, reg1);
    printk_debug("%d:  ds = %X\n", round, reg2);
    printk_debug("%d:  es = %X\n", round, reg3);
    printk_debug("%d:  ss = %X\n", round, reg4);
    ++round;
}

void panic(const char *msg) {
    printk("*** System panic: %s\n", msg);
    print_stack(10);
    printk("***\n");

    // 致命错误发生后打印栈信息后停止在这里
    while (1)
        cpu_hlt();
}

// 输出 esp
void print_stack(size_t count) {
    register addr_t *esp __asm__("esp");
    for (size_t i = 0; i < count; i++) {
        printk_debug("esp 0x%08X [0x%08X]\n", esp + i, *(esp + i));
    }
    return;
}

#ifdef __cplusplus
}
#endif

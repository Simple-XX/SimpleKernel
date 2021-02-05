
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// debug.cpp for Simple-XX/SimpleKernel.

#include "io.h"
#include "intr.h"
#include "cpu.hpp"
#include "debug.h"

DEBUG::DEBUG(void) {
    return;
}

DEBUG::~DEBUG(void) {
    return;
}

void DEBUG::init(addr_t magic __attribute__((unused)),
                 addr_t addr __attribute__((unused))) {
    io.printf("debug_init\n");
    return;
}

void DEBUG::print_cur_status(void) {
    static uint32_t round = 0;
    uint16_t        reg1, reg2, reg3, reg4;
    asm volatile("mov %%cs, %0\n\t"
                 "mov %%ds, %1\n\t"
                 "mov %%es, %2\n\t"
                 "mov %%ss, %3\n\t"
                 : "=m"(reg1), "=m"(reg2), "=m"(reg3), "=m"(reg4));

    // 打印当前的运行级别
    io.printf("%d:  @ring %d\n", round, reg1 & 0x3);
    io.printf("%d:  cs = %X\n", round, reg1);
    io.printf("%d:  ds = %X\n", round, reg2);
    io.printf("%d:  es = %X\n", round, reg3);
    io.printf("%d:  ss = %X\n", round, reg4);
    ++round;
}

void DEBUG::panic(const char *msg) {
    io.printf("*** System panic: %s\n", msg);
    print_stack(10);
    io.printf("***\n");

    // 致命错误发生后打印栈信息后停止在这里
    while (1) {
        CPU::hlt();
    }
}

// 输出 esp
void DEBUG::print_stack(size_t count) {
    register addr_t *esp __asm__("esp");
    for (size_t i = 0; i < count; i++) {
        io.printf("esp 0x%08X [0x%08X]\n", esp + i, *(esp + i));
    }
    return;
}

DEBUG debug;

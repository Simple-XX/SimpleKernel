
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "assert.h"
#include "kernel.h"

extern "C" void kernel_main(uint32_t magic, void *addr);

// 内核入口
void kernel_main(uint32_t magic, void *addr) {
    KERNEL kernel(magic, addr);
    kernel.show_info();
    CPU::sti();
    // asm("int $0x04");
    while (1) {
        ;
    }
    assert(0);
    return;
}

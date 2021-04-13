
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "assert.h"
#include "multiboot2.h"
#include "kernel.h"
extern "C" void kernel_main(uint32_t size, void *addr);

// 内核入口
void kernel_main(uint32_t size __attribute__((unused)),
                 void *   addr __attribute__((unused))) {
#if defined(__i386__) || defined(__x86_64__)
    // __x86_64__ 时用于传递 grub 信息
    MULTIBOOT2::multiboot2_init(size, addr);
#endif
    KERNEL kernel;
    kernel.show_info();
    // CPU::cli();
    while (1) {
        ;
    }
    assert(0);
    return;
}

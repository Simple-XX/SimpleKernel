
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "kernel.hpp"
#include "cpu.hpp"

extern "C" void kernel_main(void);

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

    CPU::sti();
    while (1) {
        ;
    }
    return;
}

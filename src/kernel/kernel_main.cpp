
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "kernel.h"
#include "io.h"

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

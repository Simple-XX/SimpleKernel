
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

    // enable intr
    CPU::sti();
    kernel.io.printf("sti\n");
    if (CPU::EFLAGS_IF_status()) {
        kernel.io.printf("interrupt accept!\n");
    }
    else {
        kernel.io.printf("interrupt not accept!\n");
    }

    // close intr
    CPU::cli();
    kernel.io.printf("cli\n");
    if (CPU::EFLAGS_IF_status()) {
        kernel.io.printf("interrupt accept!\n");
    }
    else {
        kernel.io.printf("interrupt not accept!\n");
    }

    CPU::sti();
    while (1) {
        ;
    }
    return;
}

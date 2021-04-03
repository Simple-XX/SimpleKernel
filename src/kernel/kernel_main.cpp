
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "kernel.h"
#include "io.h"

extern "C" void kernel_main(void);

void kernel_main(void) {
    IO     io;
    KERNEL kernel;
    kernel.show_info();
    // enable intr
    CPU::sti();
    io.printf("sti\n");
    if (CPU::EFLAGS_IF_status()) {
        io.printf("interrupt accept!\n");
    }
    else {
        io.printf("interrupt not accept!\n");
    }

    // close intr
    CPU::cli();
    io.printf("cli\n");
    if (CPU::EFLAGS_IF_status()) {
        io.printf("interrupt accept!\n");
    }
    else {
        io.printf("interrupt not accept!\n");
    }
    CPU::sti();
    while (1) {
        ;
    }
    return;
}


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "io.h"
#include "cpu.hpp"

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

    // enable intr
    cpu_sti();
    io.printf("sti\n");
    if (EFLAGS_IF_status()) {
        io.printf("interrupt accept!\n");
    }
    else {
        io.printf("interrupt not accept!\n");
    }

    // close intr
    cpu_cli();
    io.printf("cli\n");
    if (EFLAGS_IF_status()) {
        io.printf("interrupt accept!\n");
    }
    else {
        io.printf("interrupt not accept!\n");
    }

    cpu_sti();
    while (1) {
        ;
    }
    return;
}

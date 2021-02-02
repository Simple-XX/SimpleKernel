
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "io.h"
#include "cpu.hpp"

extern "C" void kernel_main(void);

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

    // enable intr
    cpu_sti();
    kernel.io.printf("sti\n");
    if (EFLAGS_IF_status()) {
        kernel.io.printf("interrupt accept!\n");
    }
    else {
        kernel.io.printf("interrupt not accept!\n");
    }

    // close intr
    cpu_cli();
    kernel.io.printf("cli\n");
    if (EFLAGS_IF_status()) {
        kernel.io.printf("interrupt accept!\n");
    }
    else {
        kernel.io.printf("interrupt not accept!\n");
    }

    cpu_sti();
    while (1) {
        ;
    }
    return;
}

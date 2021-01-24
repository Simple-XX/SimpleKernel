
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

    cpu_sti();
    while (1) {
        ;
    }
    return;
}

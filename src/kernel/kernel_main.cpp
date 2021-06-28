
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "common.h"
#include "stdio.h"
#include "iostream"
#include "kernel.h"

void kernel_main(void) {
    show_info();
    while (1) {
        ;
    }
    return;
}

void show_info(void) {
    info("kernel in memory start: 0x%08X, end 0x%08X\n", COMMON::kernel_start,
         COMMON::kernel_end);
    info("kernel in memory size: %d KB, %d pages\n",
         (COMMON::kernel_end - COMMON::kernel_start) / 1024,
         (COMMON::kernel_end - COMMON::kernel_start + 4095) / 1024 / 4);
    info("kernel in memory start: 0x%08X\n", COMMON::kernel_start);
    std::cout << "Simple Kernel." << std::endl;
    return;
}

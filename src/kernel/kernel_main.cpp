
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
    info("Kernel start: 0x%p, end 0x%p, size: 0x%p bytes, 0x%X pages\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR,
         (uint8_t *)COMMON::KERNEL_END_ADDR -
             (uint8_t *)COMMON::KERNEL_START_ADDR,
         ((uint8_t *)COMMON::KERNEL_END_ADDR -
          (uint8_t *)COMMON::KERNEL_START_ADDR) /
             4096);
    info("Kernel start4k: 0x%p, end4k: 0x%p\n", COMMON::KERNEL_START_4K,
         COMMON::KERNEL_END_4K);
    std::cout << "Simple Kernel." << std::endl;
    return;
}

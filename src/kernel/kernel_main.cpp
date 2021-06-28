
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
    info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR,
         (uint8_t *)COMMON::KERNEL_END_ADDR -
             (uint8_t *)COMMON::KERNEL_START_ADDR,
         ((uint8_t *)COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 0x1000) -
          (uint8_t *)COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 0x1000)) /
             0x1000);
    info("Kernel start4k: 0x%p, end4k: 0x%p\n",
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 0x1000),
         COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 0x1000));
    std::cout << "Simple Kernel." << std::endl;
    return;
}

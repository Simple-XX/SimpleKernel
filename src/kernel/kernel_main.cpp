
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "common.h"
#include "stdio.h"
#include "iostream"
#include "kernel.h"

// TODO: gdb 调试
// TODO: clion 环境
void kernel_main(void) {
    show_info();
    while (1) {
        ;
    }
    return;
}

void show_info(void) {
    // 内核实际大小
    auto kernel_size = COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR;
    // 内核实际占用页数
    auto kernel_pages =
        (COMMON::ALIGN(COMMON::KERNEL_END_ADDR, COMMON::PAGE_SIZE) -
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, COMMON::PAGE_SIZE)) /
        COMMON::PAGE_SIZE;
    info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages.\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR, kernel_size,
         kernel_pages);
    info("Kernel start4k: 0x%p, end4k: 0x%p.\n",
         COMMON::ALIGN(COMMON::KERNEL_START_ADDR, 4 * COMMON::KB),
         COMMON::ALIGN(COMMON::KERNEL_END_ADDR, 4 * COMMON::KB));
    std::cout << "Simple Kernel." << std::endl;
    return;
}

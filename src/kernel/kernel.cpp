
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "common.h"
#include "color.h"
#include "stdio.h"
#include "kernel.h"

KERNEL::KERNEL(void) {
    cpp_init();
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::show_info(void) {
    info("kernel in memory start: 0x%08X, end 0x%08X\n", COMMON::kernel_start,
         COMMON::kernel_end);
    info("kernel in memory size: %d KB, %d pages\n",
         (COMMON::kernel_end - COMMON::kernel_start) / 1024,
         (COMMON::kernel_end - COMMON::kernel_start + 4095) / 1024 / 4);
    printf("kernel in memory start: 0x%d\n", 0xEF);
    info("Simple Kernel.\n");
    return;
}

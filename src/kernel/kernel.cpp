
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "common.h"
#include "color.h"
#include "kernel.h"

IO KERNEL::io;

KERNEL::KERNEL(void) {
    cpp_init();
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::show_info(void) {
    // BUG: raspi2 下不能正常输出链接脚本中的地址
    // io.info("kernel in memory start: 0x%08X, end 0x%08X\n",
    //         COMMON::kernel_start, COMMON::kernel_end);
    // io.info("kernel in memory size: %d KB, %d pages\n",
    //         (COMMON::kernel_end - COMMON::kernel_start) / 1024,
    //         (COMMON::kernel_end - COMMON::kernel_start + 4095) / 1024 / 4);
    io.printf("kernel in memory start: 0x%d\n", 0xcd);
    io.info("Simple Kernel.\n");
    return;
}

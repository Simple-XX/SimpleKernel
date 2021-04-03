
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

#if defined(__i386__) || defined(__x86_64__)
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#elif defined(__arm__) || defined(__aarch64__)
#endif

IO KERNEL::io;

KERNEL::KERNEL(void) {
    cpp_init();
    arch_init();
    drv_init();
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::arch_init(void) const {
#if defined(__i386__) || defined(__x86_64__)
    GDT::init();
    INTR::init();
#elif defined(__arm__) || defined(__aarch64__)
#endif
    return;
}

void KERNEL::drv_init(void) const {
#if defined(__i386__) || defined(__x86_64__)
    clock.init();
#elif defined(__arm__) || defined(__aarch64__)
#endif
    return;
}

void KERNEL::show_info(void) {
    // BUG: raspi2 下不能正常输出链接脚本中的地址
    io.printf(COLOR::LIGHT_GREEN,
              "kernel in memory start: 0x%08X, end 0x%08X\n", kernel_start,
              kernel_end);
    io.printf(COLOR::LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
              (kernel_end - kernel_start) / 1024,
              (kernel_end - kernel_start + 4095) / 1024 / 4);
    io.printf(COLOR::LIGHT_GREEN, "Simple Kernel.\n");
    return;
}

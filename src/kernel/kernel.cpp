
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "color.h"
#include "debug.h"

#if defined(__i386__) || defined(__x86_64__)
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#elif defined(__arm__) || defined(__aarch64__)
#endif

#include "io.h"
#include "keyboard.h"
#include "kernel.h"

KERNEL::KERNEL(void) {
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::arch_init(void) const {
    GDT::init();
    INTR::init();
    return;
}

void KERNEL::show_info(void) {
    // BUG: raspi2 下不能正常输出链接脚本中的地址
    io.printf(LIGHT_GREEN, "kernel in memory start: 0x%08X, end 0x%08X\n",
              kernel_start, kernel_end);
    io.printf(LIGHT_GREEN, "kernel in memory size: %d KB, %d pages\n",
              (kernel_end - kernel_start) / 1024,
              (kernel_end - kernel_start + 4095) / 1024 / 4);
    io.printf(LIGHT_GREEN, "Simple Kernel.\n");
    return;
}

int32_t KERNEL::init(void) {
    cpp_init();
    io.init();
    arch_init();
    clock.init();
    keyboard.init();
    show_info();
    return 0;
}

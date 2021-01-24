
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "cxxabi.h"
#include "color.h"
#include "io.h"
#include "cpu.hpp"
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#include "keyboard.h"

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
    // 输出一些基本信息
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
    arch_init();
    clockk.init();
    keyboardk.init();
    this->show_info();
    return 0;
}

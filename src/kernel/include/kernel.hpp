
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.hpp for Simple-XX/SimpleKernel.

#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_

#pragma once

#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "cxxabi.h"
#include "color.h"
#include "io.hpp"
#include "debug.h"

#if defined(__i386__) || defined(__x86_64__)
#include "gdt.h"
#include "intr.h"
#endif

#include "clock.h"

extern "C" uint8_t kernel_start[];
extern "C" uint8_t kernel_text_start[];
extern "C" uint8_t kernel_text_end[];
extern "C" uint8_t kernel_data_start[];
extern "C" uint8_t kernel_data_end[];
extern "C" uint8_t kernel_end[];

class KERNEL {
private:
    void arch_init(void) const;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t init(void);
    void    show_info(void);
};

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
    io.init();
    clock.init();
    show_info();
    return 0;
}

#endif /* _KERNEL_HPP_ */

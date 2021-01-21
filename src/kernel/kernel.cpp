
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "cxxabi.h"
#include "io.h"
#include "cpu.hpp"
#include "gdt.h"
#include "intr.h"

#if defined(RASPI2)
#include "uart.h"
#include "framebuffer.h"
#endif

KERNEL::KERNEL(void) {
    return;
}

KERNEL::~KERNEL(void) {
    return;
}

void KERNEL::arch_init(void) const {
    gdtk.init();
    intrk.init();
    return;
}

void KERNEL::show_info(void) {
    // 输出一些基本信息
    iok.printf("Simple Kernel\n");
    iok.printf(
        "kernel in memory(VMA=LMA-0xC0000000) start: 0x%08X, end 0x%08X\n",
        kernel_start, kernel_end);
    iok.printf("kernel in memory size: %d KB, %d pages\n ",
               (kernel_end - kernel_start) / 1024,
               (kernel_end - kernel_start) / 1024 / 4);
    return;
}

int32_t KERNEL::init(void) {
    cpp_init();
    arch_init();
    return 0;
}

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

    cpu_sti();
    iok.printf("sti\n");
    if (EFLAGS_IF_status()) {
        iok.printf("interrupt accept!\n");
    }
    else {
        iok.printf("interrupt not accept!\n");
    }

    // close intr
    cpu_cli();
    for (int i = 0; i < 5; i++) {
        if (EFLAGS_IF_status()) {
            iok.printf("before hlt, interrupt accept!\n");
        }
        else {
            iok.printf("before hlt, interrupt not accept!\n");
        }

        cpu_hlt();

        if (EFLAGS_IF_status()) {
            iok.printf("after hlt, interrupt accept!\n\n");
        }
        else {
            iok.printf("after hlt, interrupt not accept!\n\n");
        }
    }
    // cpu_hlt();
    // cpu_cli();

    if (EFLAGS_IF_status()) {
        iok.printf("interrupt accept!\n");
    }
    else {
        iok.printf("interrupt not accept!\n");
    }

    iok.printf("\nEnd.\n");

#if defined(RASPI2)
    uart_init();
    char  c = '!';
    char *s = "gg";
    framebuffer_init();
    log_info("__%c__%s\n", c, s);
    // TODO: width 和 height 的值会出错
    for (uint32_t i = 0; i < FRAMEBUFFER_HEIGHT; i++) {
        for (uint32_t j = 0; j < FRAMEBUFFER_WIDTH; j++) {
            framebuffer_set_pixel(j, i, 0xAA00FF);
        }
    }
    log_info("Simple Kernel.\n");
#endif
    kernel.show_info();
    while (1) {
        ;
    }
    return;
}

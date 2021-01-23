
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "cxxabi.h"
#include "io.h"
#include "console.h"

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

void KERNEL::show_info(void) {
    io.printf("kernel in memory start: 0x%08X, end 0x%08X\n", kernel_start,
              kernel_end);
    io.printf("kernel in memory size: %d KB, %d pages\n",
              (kernel_end - kernel_start) / 1024,
              (kernel_end - kernel_start) / 1024 / 4);
    return;
}

int32_t KERNEL::init(void) {
    cpp_init();
    return 0;
}

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

    char        c = '!';
    const char *s = "gg\n";
    io.put_char(c);
    io.write_string(s);
    io.printf("Simple Kernel.\n");

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

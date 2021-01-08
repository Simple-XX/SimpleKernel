
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"

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
    // 输出一些基本信息
    printk("SimpleKernel\n");

    printk("kernel in memory(VMA=LMA-0xC0000000) start: 0x%08X, end 0x%08X\n",
           kernel_start, kernel_end);
    printk("kernel in memory size: %d KB, %d pages\n",
           (kernel_end - kernel_start) / 1024,
           (kernel_end - kernel_start) / 1024 / 4);
}

int32_t KERNEL::init(void) {
    char        c = '!';
    const char *s = "gg\n";
    console.put_char(c);
    console.write_string(s);
    console.printk("Simple Kernel.\n");
    return 0;
}

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

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
    return;
}

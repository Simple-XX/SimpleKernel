
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "kernel.h"
#include "console.h"
#include "uart.h"
#include "framebuffer.h"

void kernel_main(void) {
#ifdef x86_64
    // 控制台初始化
    console_init();
    char  c = '!';
    char *s = "gg";
    printk("__%c__%s\n", c, s);
    printk("Simple Kernel.\n");
#elif RASPI2
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

#ifdef __cplusplus
}
#endif

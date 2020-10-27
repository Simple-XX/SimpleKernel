
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
    log_info("__%c__%s\n", c, s);
    log_info("Simple Kernel.\n");
    while (1) {
        uart_putc(uart_getc());
    }
#endif
    return;
}

#ifdef __cplusplus
}
#endif


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/kernel.h"

void kernel_main(void) {
    // 控制台初始化
    console_init();
    char  c = '!';
    char *s = "gg";
    printk("__%c__%s\n", c, s);
    printk("Simple Kernel\n");
    return;
}

#ifdef __cplusplus
}
#endif

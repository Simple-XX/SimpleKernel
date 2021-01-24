
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"
#include "io.h"

void kernel_main(void) {
    KERNEL kernel;
    kernel.init();

#if defined(i386) || defined(x86_64)
    char        c = '!';
    const char *s = "gg\n";
    io.put_char(c);
    io.write_string(s);
    io.printf("Simple Kernel.\n");
#endif

#if defined(__arm__) || defined(__aarch__)
    char  c = '!';
    char *s = "gg";
    framebuffer_init();
    io.log("__%c__%s\n", c, s);
    // TODO: width 和 height 的值会出错
    // for (uint32_t i = 0; i < FRAMEBUFFER_HEIGHT; i++) {
    //     for (uint32_t j = 0; j < FRAMEBUFFER_WIDTH; j++) {
    //         framebuffer_set_pixel(j, i, 0xAA00FF);
    //     }
    // }
    io.log("Simple Kernel.\n");
#endif
    while (1) {
        ;
    }
    return;
}

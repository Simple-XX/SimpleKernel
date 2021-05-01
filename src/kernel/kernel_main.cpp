
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cxxabi.h"
#include "kernel.h"

extern "C" void kernel_main(void);

void kernel_main(void) {
    // 在 cpp_init 中，全局变量会进行构造
    // 包括 IO
    cpp_init();
    KERNEL kernel;
    kernel.show_info();
    while (1) {
        ;
    }
    return;
}

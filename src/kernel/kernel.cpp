
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.cpp for Simple-XX/SimpleKernel.

#include "kernel.h"

KERNEL::KERNEL(void) {
    asm("hlt");
    return;
}

KERNEL::~KERNEL() {
    return;
}


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel.h"

void kernel_main(void) {
    KERNEL k;
    k.k();
    return;
}

#ifdef __cplusplus
}
#endif

KERNEL::KERNEL(void) {
    return;
}

KERNEL::~KERNEL() {
    return;
}

int KERNEL::k() {
    return 233;
}
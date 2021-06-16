
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"

void dtb(uint32_t, uint64_t _addr) {
    printf("addr: 0x%p\n", _addr);
    return;
}

#ifdef __cplusplus
}
#endif

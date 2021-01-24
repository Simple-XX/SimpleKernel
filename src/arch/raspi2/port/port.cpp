
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.cpp for Simple-XX/SimpleKernel.

#include "port.h"

uint32_t PORT::ind(uint32_t reg) {
    return *(volatile uint32_t *)reg;
}

void PORT::outd(uint32_t reg, uint32_t data) {
    *(volatile uint32_t *)reg = data;
}

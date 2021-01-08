
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.cpp for Simple-XX/SimpleKernel.

#include "port.h"

PORT::PORT(void) {
    return;
}

PORT::~PORT(void) {
    return;
}

void PORT::outb(const uint32_t port, uint8_t data) {
    __asm__ volatile("outb %1, %0" : : "dN"(port), "a"(data));
}

uint8_t PORT::inb(const uint32_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

uint16_t PORT::inw(const uint32_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

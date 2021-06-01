
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.cpp for Simple-XX/SimpleKernel.

#include "port.h"

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

uint32_t PORT::ind(const uint32_t port) {
    uint32_t ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

void PORT::outw(const uint32_t port, const uint16_t data) {
    __asm__ volatile("outw %1, %0" : : "dN"(port), "a"(data));
    return;
}

void PORT::outb(const uint32_t port, const uint8_t data) {
    __asm__ volatile("outb %1, %0" : : "dN"(port), "a"(data));
    return;
}

void PORT::outd(const uint32_t port, const uint32_t data) {
    __asm__ volatile("outl %1, %0" : : "dN"(port), "a"(data));
    return;
}


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// port.cpp for Simple-XX/SimpleKernel.

#include "port.h"

uint8_t PORT::inb(const uint32_t port __attribute((unused))) {
    return 0;
}

uint16_t PORT::inw(const uint32_t port __attribute((unused))) {
    return 0;
}

uint32_t PORT::ind(const uint32_t port) {
    return *(volatile uint32_t *)port;
}

void PORT::outw(const uint32_t port __attribute((unused)),
                const uint16_t data __attribute((unused))) {
    return;
}

void PORT::outb(const uint32_t port __attribute((unused)),
                const uint8_t  data __attribute((unused))) {
    return;
}

void PORT::outd(const uint32_t port, const uint32_t data) {
    *(volatile uint32_t *)port = data;
    return;
}

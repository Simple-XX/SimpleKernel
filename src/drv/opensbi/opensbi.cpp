
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// opensbi.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "opensbi.h"

OPENSBI::OPENSBI(void) {
    return;
}

OPENSBI::~OPENSBI(void) {
    return;
}

uint64_t OPENSBI::ecall(uint64_t _num, uint64_t _a0, uint64_t _a1, uint64_t _a2,
                        uint64_t _a3) {
    register uint64_t a0 __asm__("a0") = (uint64_t)(_a0);
    register uint64_t a1 __asm__("a1") = (uint64_t)(_a1);
    register uint64_t a2 __asm__("a2") = (uint64_t)(_a2);
    register uint64_t a3 __asm__("a3") = (uintptr_t)(_a3);
    register uint64_t a7 __asm__("a7") = (uint64_t)(_num);
    __asm__ volatile("ecall"
                     : "+r"(a0)
                     : "r"(a1), "r"(a2), "r"(a3), "r"(a7)
                     : "memory");
    return a0;
}

void OPENSBI::put_char(const char _c) {
    ecall(CONSOLE_PUTCHAR, _c, 0, 0, 0);
    return;
}

uint8_t OPENSBI::get_char(void) {
    return ecall(CONSOLE_GETCHAR, 0, 0, 0, 0);
}

void OPENSBI::set_timer(uint64_t _value) {
    ecall(SET_TIMER, _value, 0, 0, 0);
}

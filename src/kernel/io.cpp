
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdarg.h"
#include "string.h"
#ifndef __riscv
#include "port.h"
#endif
#include "io.h"
#include "stdio.h"

static char buf[128];

#if defined(__i386__) || defined(__x86_64__)
TUI IO::io;
#elif defined(__arm__) || defined(__aarch64__)
UART IO::io;
#elif defined(__riscv)
SBI_CONSOLE IO::io;
#endif

IO::IO(void) {
    return;
}

IO::~IO(void) {
    return;
}

#ifndef __riscv
uint8_t IO::inb(const uint32_t port) {
    return PORT::inb(port);
}

uint16_t IO::inw(const uint32_t port) {
    return PORT::inw(port);
}

uint32_t IO::ind(const uint32_t port) {
    return PORT::ind(port);
}

void IO::outb(const uint32_t port, const uint8_t data) {
    PORT::outb(port, data);
    return;
}

void IO::outw(const uint32_t port, const uint16_t data) {
    PORT::outw(port, data);
    return;
}

void IO::outd(const uint32_t port, const uint32_t data) {
    PORT::outd(port, data);
    return;
}

#endif

COLOR::color_t IO::get_color(void) {
    return io.get_color();
}

void IO::set_color(const COLOR::color_t color) {
    io.set_color(color);
    return;
}

void IO::put_char(char c) {
    io.put_char(c);
    return;
}

char IO::get_char(void) {
    return io.get_char();
}

int32_t IO::write_string(const char *s) {
    io.write_string(s);
    return 0;
}

extern "C" int32_t printf(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    const int ret = _vsnprintf(buf, 127, fmt, va);
    va_end(va);
    io.write_string(buf);
    bzero(buf, 128);
    return ret;
}

extern "C" int32_t info(const char *fmt, ...) {
    COLOR::color_t curr_color = io.get_color();
    io.set_color(COLOR::LIGHT_GREEN);
    va_list va;
    int32_t i;
    va_start(va, fmt);
    i = vsnprintf_(buf, (size_t)-1, fmt, va);
    va_end(va);
    io.write_string(buf);
    bzero(buf, 128);
    io.set_color(curr_color);
    return i;
}

extern "C" int32_t warn(const char *fmt, ...) {
    COLOR::color_t curr_color = io.get_color();
    io.set_color(COLOR::LIGHT_MAGENTA);
    va_list va;
    int32_t i;
    va_start(va, fmt);
    i = vsnprintf_(buf, (size_t)-1, fmt, va);
    va_end(va);
    io.write_string(buf);
    bzero(buf, 128);
    io.set_color(curr_color);
    return i;
}

extern "C" int32_t err(const char *fmt, ...) {
    COLOR::color_t curr_color = io.get_color();
    io.set_color(COLOR::LIGHT_RED);
    va_list va;
    int32_t i;
    va_start(va, fmt);
    i = vsnprintf_(buf, (size_t)-1, fmt, va);
    va_end(va);
    io.write_string(buf);
    bzero(buf, 128);
    io.set_color(curr_color);
    return i;
}

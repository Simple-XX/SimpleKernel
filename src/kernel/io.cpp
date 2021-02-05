
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdarg.h"
#include "string.h"
#include "port.h"
#include "io.h"

extern "C" int32_t vsprintf(char *buf, const char *fmt, va_list args);

char IO::buf[128];

#if defined(__i386__) || defined(__x86_64__)
VGA IO::io;
#elif defined(__arm__) || defined(__aarch64__)
UART IO::io;
#endif

IO::IO(void) {
    return;
}

IO::~IO(void) {
    return;
}

int32_t IO::init(void) {
    io.init();
    write_string("io init\n");
    return 0;
}

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

color_t IO::get_color(void) {
    return io.get_color();
}

void IO::set_color(const color_t color) {
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

int32_t IO::printf(const char *fmt, ...) {
    va_list args;
    int32_t i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    write_string(buf);
    bzero(buf, 128);
    return i;
}

int32_t IO::printf(color_t color, const char *fmt, ...) {
    color_t curr_color = get_color();
    set_color(color);
    va_list args;
    int32_t i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    write_string(buf);
    bzero(buf, 128);
    set_color(curr_color);
    return i;
}

IO io;

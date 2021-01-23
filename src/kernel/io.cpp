
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.cpp for Simple-XX/SimpleKernel.

#include "string.h"
#include "io.h"
#include "port.h"
#include "console.h"

extern "C" int32_t vsprintf(char *buf, const char *fmt, va_list args);

IO::IO(void) {
    return;
}

IO::~IO(void) {
    return;
}

uint8_t IO::inb(const uint32_t port) {
    return PORT::inb(port);
}

uint16_t IO::inw(const uint32_t port) {
    return PORT::inw(port);
}

void IO::outb(const uint32_t port, const uint8_t data) {
    PORT::outb(port, data);
    return;
}

void IO::outw(const uint32_t port, const uint16_t data) {
    PORT::outw(port, data);
    return;
}

size_t IO::get_rows(void) {
    return 0;
}

size_t IO::get_clos(void) {
    return 0;
}

void IO::set_rows(const size_t rows __attribute__((unused))) {
    return;
}

void IO::set_cols(const size_t cols __attribute__((unused))) {
    return;
}

size_t IO::get_row(void) {
    return 0;
}

size_t IO::get_col(void) {
    return consolek.get_color();
}

size_t IO::set_row(const size_t row __attribute__((unused))) {
    return 0;
}

size_t IO::set_col(const size_t col __attribute__((unused))) {
    return 0;
}

color_t IO::get_color(void) {
    return consolek.get_color();
}

void IO::set_color(const color_t color) {
    consolek.set_color(color);
    return;
}

void IO::put_char(char c) {
    consolek.put_char(c);
    return;
}

int32_t IO::write_string(const char *s) {
    consolek.write_string(s);
    return 0;
}

int32_t IO::printf(const char *fmt, ...) {
    va_list args;
    int32_t i;
    char    buf[256];
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    consolek.write_string(buf);
    bzero(buf, 256);
    return i;
}

int32_t IO::printf(color_t color, const char *fmt, ...) {
    color_t curr_color = this->get_color();
    this->set_color(color);
    va_list args;
    int32_t i;
    char    buf[256];
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    consolek.write_string(buf);
    bzero(buf, 256);
    this->set_color(curr_color);
    return i;
}

IO io;


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.hpp for Simple-XX/SimpleKernel.

#ifndef _IO_HPP_
#define _IO_HPP_

#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "stddef.h"
#include "color.h"
#include "port.h"

#if defined(__i386__) || defined(__x86_64__)
#include "vga.h"
#endif

extern "C" int32_t vsprintf(char *buf, const char *fmt, va_list args);

template <class io_t>
class IO {
private:
    // io 缓冲
    static char buf[128];
    // io 对象
    static io_t io;

protected:
public:
    IO(void);
    ~IO(void);
    int32_t init(void);
    // 端口读字节
    uint8_t inb(const uint32_t port);
    // 端口读字
    uint16_t inw(const uint32_t port);
    // 端口读双字
    uint32_t ind(const uint32_t port);
    // 端口写字节
    void outb(const uint32_t port, const uint8_t data);
    // 端口写字
    void outw(const uint32_t port, const uint16_t data);
    // 端口写双字
    void outd(const uint32_t port, const uint32_t data);
    // 获取当前颜色
    color_t get_color(void);
    // 设置当前颜色
    void set_color(const color_t color);
    // 输出字符
    void put_char(const char c);
    // 输出字符串
    int32_t write_string(const char *s);
    // 格式输出
    int32_t printf(const char *fmt, ...);
    int32_t printf(color_t color, const char *fmt, ...);
};

template <class io_t>
char IO<io_t>::buf[128];

template <class io_t>
io_t IO<io_t>::io;

template <class io_t>
IO<io_t>::IO(void) {
    return;
}

template <class io_t>
IO<io_t>::~IO(void) {
    return;
}

template <class io_t>
int32_t IO<io_t>::init(void) {
    write_string("io init\n");
    return 0;
}

template <class io_t>
uint8_t IO<io_t>::inb(const uint32_t port) {
    return PORT::inb(port);
}

template <class io_t>
uint16_t IO<io_t>::inw(const uint32_t port) {
    return PORT::inw(port);
}

template <class io_t>
uint32_t IO<io_t>::ind(const uint32_t port) {
    return PORT::ind(port);
}

template <class io_t>
void IO<io_t>::outb(const uint32_t port, const uint8_t data) {
    PORT::outb(port, data);
    return;
}

template <class io_t>
void IO<io_t>::outw(const uint32_t port, const uint16_t data) {
    PORT::outw(port, data);
    return;
}

template <class io_t>
void IO<io_t>::outd(const uint32_t port, const uint32_t data) {
    PORT::outd(port, data);
    return;
}

template <class io_t>
color_t IO<io_t>::get_color(void) {
    return io.get_color();
}

template <class io_t>
void IO<io_t>::set_color(const color_t color) {
    io.set_color(color);
    return;
}

template <class io_t>
void IO<io_t>::put_char(char c) {
    io.put_char(c);
    return;
}

template <class io_t>
int32_t IO<io_t>::write_string(const char *s) {
    io.write_string(s);
    return 0;
}

template <class io_t>
int32_t IO<io_t>::printf(const char *fmt, ...) {
    va_list args;
    int32_t i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    write_string(buf);
    bzero(buf, 128);
    return i;
}

template <class io_t>
int32_t IO<io_t>::printf(color_t color, const char *fmt, ...) {
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

#endif /* _IO_HPP_ */

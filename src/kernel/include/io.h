
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.h for Simple-XX/SimpleKernel.

#ifndef _IO_H_
#define _IO_H_

#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "stddef.h"
#include "color.h"
#include "port.h"

#if defined(__i386__) || defined(__x86_64__)
#include "vga.h"
#endif

#include "console.h"

extern "C" int32_t vsprintf(char *buf, const char *fmt, va_list args);

template <class video_t>
class IO {
private:
    // //屏幕缓冲区
    // uint8_t *screen;
    // //用于保存历史记录
    // uint8_t screen[SIZESCREEN];
    // // console 缓冲
    // uint8_t buf[512];
    // // console read position
    // int keypos;
    // // console 状态
    // int inlock;
    // // console foreground color
    // uint8_t fcolor;
    // // console background color
    // uint8_t bcolor;
    // 当前行
    size_t curr_row;
    // 当前列
    size_t curr_col;
    // // console 属性
    // uint8_t kattr;
    // // 显存地址
    // static char *vidmem;
    CONSOLE<video_t> &console;

protected:
public:
    IO(CONSOLE<video_t> &console);
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
    // 获取行数
    size_t get_rows(void);
    // 获取列数
    size_t get_clos(void);
    // 设置行数
    void set_rows(const size_t rows);
    // 设置列数
    void set_cols(const size_t cols);
    // 获取当前行
    size_t get_row(void);
    // 获取当前列
    size_t get_col(void);
    // 设置当前行
    size_t set_row(const size_t row);
    // 设置当前列
    size_t set_col(const size_t col);
    // 获取当前颜色
    color_t get_color(void);
    // 设置当前颜色
    void set_color(const color_t color);
    // 输出字符
    void put_char(const char c);
    // 输出字符串
    int32_t write_string(const char *s);
    // 串口输出
    int32_t log(const char *format, ...);
    // 格式输出
    int32_t printf(const char *fmt, ...);
    int32_t printf(color_t color, const char *fmt, ...);
};

template <class video_t>
IO<video_t>::IO(CONSOLE<video_t> &console) : console(console) {
    return;
}

template <class video_t>
IO<video_t>::~IO(void) {
    return;
}

template <class video_t>
int32_t IO<video_t>::init(void) {
    write_string("io init\n");
    return 0;
}

template <class video_t>
uint8_t IO<video_t>::inb(const uint32_t port) {
    return PORT::inb(port);
}

template <class video_t>
uint16_t IO<video_t>::inw(const uint32_t port) {
    return PORT::inw(port);
}

template <class video_t>
uint32_t IO<video_t>::ind(const uint32_t port) {
    return PORT::ind(port);
}

template <class video_t>
void IO<video_t>::outb(const uint32_t port, const uint8_t data) {
    PORT::outb(port, data);
    return;
}

template <class video_t>
void IO<video_t>::outw(const uint32_t port, const uint16_t data) {
    PORT::outw(port, data);
    return;
}

template <class video_t>
void IO<video_t>::outd(const uint32_t port, const uint32_t data) {
    PORT::outd(port, data);
    return;
}

template <class video_t>
size_t IO<video_t>::get_rows(void) {
    return 0;
}

template <class video_t>
size_t IO<video_t>::get_clos(void) {
    return 0;
}

template <class video_t>
void IO<video_t>::set_rows(const size_t rows) {
    console.set_rows(rows);
    return;
}

template <class video_t>
void IO<video_t>::set_cols(const size_t cols) {
    console.set_cols(cols);
    return;
}

template <class video_t>
size_t IO<video_t>::get_row(void) {
    return 0;
}

template <class video_t>
size_t IO<video_t>::get_col(void) {
    return 0;
}

template <class video_t>
size_t IO<video_t>::set_row(const size_t row) {
    curr_row = row;
    console.set_row(row);
    return 0;
}

template <class video_t>
size_t IO<video_t>::set_col(const size_t col) {
    curr_col = col;
    console.set_col(col);
    return 0;
}

template <class video_t>
color_t IO<video_t>::get_color(void) {
    return console.get_color();
}

template <class video_t>
void IO<video_t>::set_color(const color_t color) {
    console.set_color(color);
    return;
}

template <class video_t>
void IO<video_t>::put_char(char c) {
    console.put_char(c);
    return;
}

template <class video_t>
int32_t IO<video_t>::write_string(const char *s) {
    console.write_string(s);
    return 0;
}

template <class video_t>
int32_t IO<video_t>::printf(const char *fmt, ...) {
    va_list args;
    int32_t i;
    char    buf[256];
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    write_string(buf);
    bzero(buf, 256);
    return i;
}

template <class video_t>
int32_t IO<video_t>::printf(color_t color, const char *fmt, ...) {
    color_t curr_color = get_color();
    set_color(color);
    va_list args;
    int32_t i;
    char    buf[256];
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    write_string(buf);
    bzero(buf, 256);
    set_color(curr_color);
    return i;
}

#endif /* _IO_H_ */

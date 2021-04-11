
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.h for Simple-XX/SimpleKernel.

#ifndef _IO_H_
#define _IO_H_

#include "stdint.h"
#include "color.h"

#if defined(__i386__) || defined(__x86_64__)
#include "tui.h"
#elif defined(__arm__) || defined(__aarch64__)
#include "uart.h"
#endif

class IO {
private:
    // io 缓冲
    static char buf[128];
    // io 对象
#if defined(__i386__) || defined(__x86_64__)
    static TUI io;
#elif defined(__arm__) || defined(__aarch64__)
    static UART io;
#endif

protected:
public:
    IO(void);
    ~IO(void);
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
    COLOR::color_t get_color(void);
    // 设置当前颜色
    void set_color(const COLOR::color_t color);
    // 输出字符
    void put_char(const char c);
    // 输入字符
    char get_char(void);
    // 输出字符串
    int32_t write_string(const char *s);
    // 格式输出
    int32_t printf(const char *fmt, ...);
    int32_t info(const char *fmt, ...);
    int32_t warn(const char *fmt, ...);
    int32_t err(const char *fmt, ...);
};

#endif /* _IO_H_ */


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// io.h for Simple-XX/SimpleKernel.

#ifndef _IO_H_
#define _IO_H_

#include "stdint.h"
#include "stddef.h"
#include "color.h"

#if defined(__i386__) || defined(__x86_64__)
#include "vga.h"
#endif

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
    // // 当前行
    // uint8_t curr_row;
    // // 当前列
    // uint8_t curr_col;
    // // console 属性
    // uint8_t kattr;
    // // 显存地址
    // static char *vidmem;

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
    // 格式输出
    int32_t printf(const char *format, ...);
    int32_t printf(color_t color, const char *format, ...);
    // 串口输出
    int32_t log(const char *format, ...);
};

// 内核 IO
extern IO io;

#endif /* _IO_H_ */

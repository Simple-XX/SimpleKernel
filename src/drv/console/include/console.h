
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// console.h for Simple-XX/SimpleKernel.

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "vga.h"

class CONSOLE {
private:
    // 命令行行数
    size_t rows;
    // 命令行列数
    size_t cols;
    // 当前位置
    size_t curr_row;
    size_t curr_col;
    // 当前命令行颜色
    color_t color;
    // 清屏
    void clear(void);
    // 转义字符处理
    void escapeconv(const char c);
    // 滚动显示
    void scroll(void);
    // 指定位置输出
    void put_entry_at(const char c, const color_t color, const size_t x,
                      const size_t y);

protected:
public:
    CONSOLE(void);
    ~CONSOLE(void);
    // 写字符
    void put_char(const char c);
    // 写字符串
    void write_string(const char *s);
    // 写字符串
    void write(const char *s, size_t len);
    // 设置颜色
    void set_color(const color_t color);
    // 获取颜色
    color_t get_color(void);
};

// 内核 console
extern CONSOLE consolek;

#endif /* _CONSOLE_H_ */

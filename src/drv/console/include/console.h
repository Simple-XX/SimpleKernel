
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// console.h for Simple-XX/SimpleKernel.

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "stddef.h"
#include "stdint.h"
#include "vga.h"

class CONSOLE {
private:
    VGA vga;
    // 命令行行数
    size_t rows;
    // 命令行列数
    size_t cols;
    // 当前位置
    size_t curr_row;
    size_t curr_col;
    // 当前命令行颜色
    uint8_t color;

protected:
public:
    CONSOLE(void);
    ~CONSOLE(void);
    // 清屏
    void clear(void);
    // 滚动显示
    void scroll(void);
    // 写字符串
    void write_string(const char *);
    // 写字符串
    void write(const char *, size_t);
    // 写字符
    void put_char(char);
    // 转义字符处理
    void escapeconv(char);
    // 指定位置输出
    void put_entry_at(char c, uint8_t color, size_t x, size_t y);
    // 设置颜色
    void set_color(uint8_t);
    // 获取颜色
    uint8_t get_color(void);
    // 格式输出
    static int32_t printk(const char *format, ...);
};

extern CONSOLE console;

#endif /* _CONSOLE_H_ */

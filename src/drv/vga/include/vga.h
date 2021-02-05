
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.h for Simple-XX/SimpleKernel.

#ifndef _VGA_H_
#define _VGA_H_

#pragma once

#include "stdint.h"
#include "stddef.h"
#include "color.h"

class VGA {
private:
    // CRT 控制寄存器-地址
    static constexpr const uint32_t VGA_ADDR = 0x3D4;
    // CRT 控制寄存器-数据
    static constexpr const uint32_t VGA_DATA = 0x3D5;
    // 光标高位
    static constexpr const uint32_t VGA_CURSOR_H = 0xE;
    // 光标低位
    static constexpr const uint32_t VGA_CURSOR_L = 0xF;
    // VGA 缓存基址
    const uint16_t *VGA_MEM_BASE = reinterpret_cast<const uint16_t *>(0xB8000);
    // VGA 缓存大小
    static constexpr size_t VGA_MEM_SIZE = 0x8000;
    // 规定显示行数、列数
    static constexpr const size_t WIDTH  = 80;
    static constexpr const size_t HEIGHT = 25;
    // 缓存
    uint16_t *const buffer = (uint16_t *)VGA_MEM_BASE;
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
    VGA(void);
    ~VGA(void);
    // 生成颜色，详解见 '颜色设置与位运算.md'
    // fg-font
    // bg-back
    int32_t init(void);
    color_t gen_color(const color_t fg, const color_t bg) const;
    // 生成一个字符+颜色
    // uc-字符
    // color-颜色
    uint16_t gen_char(const uint8_t uc, const color_t color) const;
    // 写缓存
    void write(const size_t idx, const uint16_t data);
    // 读缓存
    uint16_t read(const size_t idx) const;
    // 设置光标位置
    void set_cursor_pos(const size_t x, const size_t y);
    // 获取光标位置
    uint16_t get_cursor_pos(void) const;
    // 写字符
    void put_char(const char c);
    // 读字符
    char get_char(void);
    // 写字符串
    void write_string(const char *s);
    // 写字符串
    void write(const char *s, size_t len);
    // 设置颜色
    void set_color(const color_t color);
    // 获取颜色
    color_t get_color(void);
    // 设置行列
    void set_row(size_t row);
    void set_col(size_t col);
};

#endif /* _VGA_H_ */

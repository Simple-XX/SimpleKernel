
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.h for Simple-XX/SimpleKernel.

#ifndef _VGA_H_
#define _VGA_H_

#include "stdint.h"
#include "stddef.h"
#include "port.h"

// vga 颜色
typedef enum {
    BLACK         = 0,
    BLUE          = 1,
    GREEN         = 2,
    CYAN          = 3,
    RED           = 4,
    MAGENTA       = 5,
    BROWN         = 6,
    LIGHT_GREY    = 7,
    DARK_GREY     = 8,
    LIGHT_BLUE    = 9,
    LIGHT_GREEN   = 10,
    LIGHT_CYAN    = 11,
    LIGHT_RED     = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_BROWN   = 14,
    WHITE         = 15,
} color_t;

class VGA {
private:
    // CRT 控制寄存器-地址
    const uint32_t VGA_ADDR = 0x3D4;
    // CRT 控制寄存器-数据
    const uint32_t VGA_DATA = 0x3D5;
    // 光标高位
    const uint32_t VGA_CURSOR_H = 0xE;
    // 光标低位
    const uint32_t VGA_CURSOR_L = 0xF;
    // VGA 缓存基址
    const uint16_t *VGA_MEM_BASE = (uint16_t *)0xB8000;
    // VGA 缓存大小
    const size_t VGA_MEM_SIZE = 0x8000;
    // 规定显示行数、列数
    const size_t width  = 80;
    const size_t height = 25;
    // 用于端口读写
    PORT port;

protected:
    // 缓存
    uint16_t *buffer = (uint16_t *)VGA_MEM_BASE;

public:
    VGA(void);
    ~VGA(void);
    // 设置颜色，详解见 '颜色设置与位运算.md'
    // fg-font
    // bg-back
    uint8_t set_color(color_t fg, color_t bg);
    // uc-字符
    // color-颜色
    uint16_t set_char_color(uint8_t uc, uint8_t color);
    // 获取高度
    size_t get_height(void);
    // 获取宽度
    size_t get_width(void);
    // 写缓存
    void write(size_t idx, uint16_t data);
    // 读缓存
    uint16_t read(size_t idx);
    // 设置光标位置
    void set_cursor_pos(size_t x, size_t y);
    // 获取光标位置
    uint16_t get_cursor_pos(void);
};

#endif /* _VGA_H_ */

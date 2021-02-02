
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.cpp for Simple-XX/SimpleKernel.

#include "vga.h"
#include "port.h"
#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "string.h"

VGA::VGA(void) {
    color    = gen_color(LIGHT_GREY, BLACK);
    curr_row = 0;
    curr_col = 0;
    return;
}

VGA::~VGA(void) {
    return;
}

int32_t VGA::init(void) {
    clear();
    this->write_string("vga init\n");
    return 0;
}

color_t VGA::gen_color(const color_t fg, const color_t bg) const {
    return (color_t)((uint8_t)fg | ((uint8_t)bg << 4));
}

uint16_t VGA::gen_char(const uint8_t uc, const color_t color) const {
    return (uint16_t)uc | (uint16_t)color << 8;
}

void VGA::write(const size_t index, const uint16_t data) {
    this->buffer[index] = data;
    return;
}

uint16_t VGA::read(size_t idx) const {
    return this->buffer[idx];
}

void VGA::set_cursor_pos(const size_t x, const size_t y) {
    const uint16_t index = y * this->WIDTH + x;
    // 光标的设置，见参考资料
    // 告诉 VGA 我们要设置光标的高字节
    PORT::outb(VGA_ADDR, VGA_CURSOR_H);
    // 发送高 8 位
    PORT::outb(VGA_DATA, index >> 8);
    // 告诉 VGA 我们要设置光标的低字节
    PORT::outb(VGA_ADDR, VGA_CURSOR_L);
    // 发送低 8 位
    PORT::outb(VGA_DATA, index);
    return;
}

uint16_t VGA::get_cursor_pos(void) const {
    PORT::outb(VGA_ADDR, VGA_CURSOR_H);
    size_t cursor_pos_h = PORT::inb(VGA_DATA);
    PORT::outb(VGA_ADDR, VGA_CURSOR_L);
    size_t cursor_pos_l = PORT::inb(VGA_DATA);
    // 返回光标位置
    return (cursor_pos_h << 8) | cursor_pos_l;
}

void VGA::put_entry_at(const char c, const color_t color, const size_t x,
                       const size_t y) {
    const size_t index = y * WIDTH + x;
    write(index, gen_char(c, color));
    return;
}

void VGA::clear(void) {
    // 从左上角开始
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            const size_t index = y * WIDTH + x;
            // 用 ' ' 填满屏幕
            // 字体为灰色，背景为黑色
            write(index, gen_char(' ', color));
        }
    }
    set_cursor_pos(0, 0);
    return;
}

void VGA::scroll(void) {
    if (curr_row >= HEIGHT) {
        // 将所有行的显示数据复制到上一行
        for (size_t i = 0; i < (HEIGHT - 1) * WIDTH; i++) {
            write(i, read(i + WIDTH));
        }
        // 最后的一行数据现在填充空格，不显示任何字符
        for (size_t i = (HEIGHT - 1) * WIDTH; i < HEIGHT * WIDTH; i++) {
            write(i, gen_char(' ', color));
        }
        // 向上移动了一行，所以 cursor_y 现在是 24
        curr_row = HEIGHT - 1;
    }
    return;
}

void VGA::write_string(const char *data) {
    write(data, strlen(data));
    return;
}

void VGA::write(const char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        put_char(data[i]);
    }
    return;
}

void VGA::put_char(const char c) {
    put_entry_at(c, color, curr_col, curr_row);
    // 如果到达最后一列则换行
    if (++curr_col >= WIDTH) {
        curr_col = 0;
        curr_row++;
    }
    // 转义字符处理
    escapeconv(c);
    // 屏幕滚动
    scroll();
    set_cursor_pos(curr_col, curr_row);
    return;
}

void VGA::escapeconv(const char c) {
    switch (c) {
        case '\n': {
            curr_col = 0;
            curr_row++;
            break;
        }
        case '\t': {
            // 取整对齐
            curr_col = (curr_col + 7) & ~7;
            // 如果到达最后一列则换行
            if (++curr_col >= WIDTH) {
                curr_col = 0;
                curr_row++;
            }
            break;
        }
        case '\b': {
            if (curr_col) {
                curr_col -= 2;
            }
            break;
        }
    }
    return;
}

void VGA::set_color(const color_t color) {
    this->color = color;
    return;
}

color_t VGA::get_color(void) {
    return this->color;
}

void VGA::set_row(size_t row) {
    curr_row = row;
    set_cursor_pos(curr_col, curr_row);
    return;
}

void VGA::set_col(size_t col) {
    curr_col = col;
    set_cursor_pos(curr_col, curr_row);
    return;
}

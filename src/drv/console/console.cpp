
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// console.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "string.h"
#include "stdarg.h"
#include "string.h"
#include "console.h"

CONSOLE::CONSOLE(void) {
    rows  = vgak.get_height();
    cols  = vgak.get_width();
    color = vgak.gen_color(LIGHT_GREY, BLACK);
    clear();
    curr_row = 0;
    curr_col = 0;
    this->write_string("console_init\n");
    return;
}

CONSOLE::~CONSOLE(void) {
    return;
}

void CONSOLE::put_entry_at(const char c, const color_t color, const size_t x,
                           const size_t y) {
    const size_t index = y * cols + x;
    vgak.write(index, vgak.gen_char(c, color));
    return;
}

void CONSOLE::clear(void) {
    // 从左上角开始
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < cols; x++) {
            const size_t index = y * cols + x;
            // 用 ' ' 填满屏幕
            // 字体为灰色，背景为黑色
            vgak.write(index, vgak.gen_char(' ', color));
        }
    }
    vgak.set_cursor_pos(0, 0);
    return;
}

void CONSOLE::scroll(void) {
    if (curr_row >= rows) {
        // 将所有行的显示数据复制到上一行
        for (size_t i = 0; i < (rows - 1) * cols; i++) {
            vgak.write(i, vgak.read(i + cols));
        }
        // 最后的一行数据现在填充空格，不显示任何字符
        for (size_t i = (rows - 1) * cols; i < rows * cols; i++) {
            vgak.write(i, vgak.gen_char(' ', color));
        }
        // 向上移动了一行，所以 cursor_y 现在是 24
        curr_row = 24;
    }
    return;
}

void CONSOLE::write_string(const char *data) {
    write(data, strlen(data));
    return;
}

void CONSOLE::write(const char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        put_char(data[i]);
    }
    return;
}

void CONSOLE::put_char(const char c) {
    put_entry_at(c, color, curr_col, curr_row);
    // 如果到达最后一列则换行
    if (++curr_col >= cols) {
        curr_col = 0;
        curr_row++;
    }
    // 转义字符处理
    escapeconv(c);
    // 屏幕滚动
    scroll();
    vgak.set_cursor_pos(curr_col, curr_row);
    return;
}

void CONSOLE::escapeconv(const char c) {
    switch (c) {
        case '\n': {
            curr_col = 0;
            curr_row++;
            break;
        }
        case '\t': { // 取整对齐
            curr_col = (curr_col + 7) & ~7;
            // 如果到达最后一列则换行
            if (++curr_col >= cols) {
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

void CONSOLE::set_color(const color_t color) {
    this->color = color;
    return;
}

color_t CONSOLE::get_color(void) {
    return this->color;
}

CONSOLE consolek;

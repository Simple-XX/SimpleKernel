
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// TUI.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "tui.h"
#include "port.h"

pos_t::pos_t(const uint8_t _col, const uint8_t _row) : col(_col), row(_row) {
    return;
}

pos_t::~pos_t(void) {
    return;
}

// See https://wiki.osdev.org/Text_UI
col_t::col_t(const COLOR::color_t _fore, const COLOR::color_t _back)
    : fore(_fore), back(_back) {
    return;
}

col_t::~col_t(void) {
    return;
}

char_t::char_t(const uint8_t _c, const col_t _color) : c(_c), color(_color) {
    return;
}

char_t::~char_t(void) {
    return;
}

// 当前位置
pos_t TUI::pos(0, 0);
// 当前命令行颜色
col_t TUI::color(COLOR::WHITE, COLOR::BLACK);

TUI::TUI(void) {
    clear();
    this->write_string("TUI init.\n");
    return;
}

TUI::~TUI(void) {
    return;
}

void TUI::put_entry_at(const char _c, const col_t _color, const size_t _x,
                       const size_t _y) {
    const size_t index = _y * WIDTH + _x;
    write(index, char_t(_c, _color));
    return;
}

bool TUI::escapeconv(const char _c) {
    switch (_c) {
        case '\n': {
            pos.row++;
            pos.col = 0;
            return true;
        }
        case '\t': {
            // 取整对齐
            pos.col += (pos.col % 4 == 0) ? 4 : 4 - (pos.col % 4);
            return true;
        }
        case '\b': {
            if (pos.col > 0) {
                write(pos.row * WIDTH + --pos.col, char_t(' ', color));
            }
            return true;
        }
    }
    return false;
}

void TUI::scroll(void) {
    if (pos.row >= HEIGHT) {
        // 将所有行的显示数据复制到上一行
        for (size_t i = 0; i < (HEIGHT - 1) * WIDTH; i++) {
            write(i, read(i + WIDTH));
        }
        // 最后的一行数据现在填充空格，不显示任何字符
        for (size_t i = (HEIGHT - 1) * WIDTH; i < HEIGHT * WIDTH; i++) {
            write(i, char_t(' ', color));
        }
        // 向上移动了一行，所以 cursor_y 现在是 24
        pos.row = HEIGHT - 1;
    }
    return;
}

void TUI::set_color(const COLOR::color_t _color) {
    this->color = col_t(_color, COLOR::BLACK);
    return;
}

COLOR::color_t TUI::get_color(void) const {
    return (COLOR::color_t)(this->color.back | this->color.fore);
}

void TUI::set_pos(const pos_t _pos) {
    const uint16_t index = _pos.row * this->WIDTH + _pos.col;
    this->pos            = _pos;
    // 光标的设置，见参考资料
    // 告诉 TUI 我们要设置光标的高字节
    PORT::outb(TUI_ADDR, TUI_CURSOR_H);
    // 发送高 8 位
    PORT::outb(TUI_DATA, index >> 8);
    // 告诉 TUI 我们要设置光标的低字节
    PORT::outb(TUI_ADDR, TUI_CURSOR_L);
    // 发送低 8 位
    PORT::outb(TUI_DATA, index);
    return;
}

void TUI::set_pos_row(const size_t _row) {
    pos.row = _row;
    set_pos(pos);
    return;
}

void TUI::set_pos_col(const size_t _col) {
    pos.col = _col;
    set_pos(pos);
    return;
}

pos_t TUI::get_pos(void) const {
    PORT::outb(TUI_ADDR, TUI_CURSOR_H);
    size_t cursor_pos_h = PORT::inb(TUI_DATA);
    PORT::outb(TUI_ADDR, TUI_CURSOR_L);
    size_t cursor_pos_l = PORT::inb(TUI_DATA);
    // 返回光标位置
    return pos_t(cursor_pos_l, cursor_pos_h);
}

void TUI::write(const size_t _idx, const char_t _data) {
    this->buffer[_idx] = _data;
    return;
}

char_t TUI::read(const size_t _idx) const {
    return this->buffer[_idx];
}

void TUI::put_char(const char _c) {
    // 转义字符处理
    if (escapeconv(_c) == false) {
        put_entry_at(_c, color, pos.col, pos.row);
        // 如果到达最后一列则换行
        if (++pos.col >= WIDTH) {
            pos.col = 0;
            pos.row++;
        }
    }
    // 屏幕滚动
    scroll();
    set_pos(pos);
    return;
}

char TUI::get_char(void) const {
    return '\0';
}

void TUI::write_string(const char *_s) {
    write(_s, strlen(_s));
    return;
}

void TUI::write(const char *_s, const size_t _len) {
    for (size_t i = 0; i < _len; i++) {
        put_char(_s[i]);
    }
    return;
}

void TUI::clear(void) {
    // 从左上角开始
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            const size_t index = y * WIDTH + x;
            // 用 ' ' 填满屏幕
            // 字体为灰色，背景为黑色
            write(index, char_t(' ', color));
        }
    }
    set_pos(pos_t(0, 0));
    return;
}

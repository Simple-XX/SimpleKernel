
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.cpp for Simple-XX/SimpleKernel.

#include "vga.h"
#include "port.h"

VGA::VGA(void) {
    return;
}

VGA::~VGA(void) {
    return;
}

int32_t VGA::init(void) {
    return 0;
}

color_t VGA::gen_color(const color_t fg, const color_t bg) const {
    return (color_t)((uint8_t)fg | ((uint8_t)bg << 4));
}

uint16_t VGA::gen_char(const uint8_t uc, const color_t color) const {
    return (uint16_t)uc | (uint16_t)color << 8;
}

size_t VGA::get_height(void) const {
    return this->height;
}

size_t VGA::get_width(void) const {
    return this->width;
}

void VGA::write(const size_t index, const uint16_t data) {
    this->buffer[index] = data;
    return;
}

uint16_t VGA::read(size_t idx) const {
    return this->buffer[idx];
}

void VGA::set_cursor_pos(const size_t x, const size_t y) {
    const uint16_t index = y * this->width + x;
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

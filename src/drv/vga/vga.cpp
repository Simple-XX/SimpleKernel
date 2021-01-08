
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.cpp for Simple-XX/SimpleKernel.

#include "vga.h"

VGA::VGA(void) : port(PORT()) {
    return;
}

VGA::~VGA(void) {
    return;
}

uint8_t VGA::set_color(color_t fg, color_t bg) {
    return fg | (bg << 4);
}

uint16_t VGA::set_char_color(uint8_t uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

size_t VGA::get_height(void) {
    return this->height;
}

size_t VGA::get_width(void) {
    return this->width;
}

void VGA::write(size_t index, uint16_t data) {
    this->buffer[index] = data;
    return;
}

uint16_t VGA::read(size_t idx) {
    return this->buffer[idx];
}

void VGA::set_cursor_pos(size_t x, size_t y) {
    const uint16_t index = y * this->width + x;
    // 光标的设置，见参考资料
    // 告诉 VGA 我们要设置光标的高字节
    port.outb(VGA_ADDR, VGA_CURSOR_H);
    // 发送高 8 位
    port.outb(VGA_DATA, index >> 8);
    // 告诉 VGA 我们要设置光标的低字节
    port.outb(VGA_ADDR, VGA_CURSOR_L);
    // 发送低 8 位
    port.outb(VGA_DATA, index);
    return;
}

uint16_t VGA::get_cursor_pos(void) {
    port.outb(VGA_ADDR, VGA_CURSOR_H);
    size_t cursor_pos_h = port.inb(VGA_DATA);
    port.outb(VGA_ADDR, VGA_CURSOR_L);
    size_t cursor_pos_l = port.inb(VGA_DATA);
    // 返回光标位置
    return (cursor_pos_h << 8) | cursor_pos_l;
}


// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vga.hpp for Simple-XX/SimpleKernel.

#ifndef _VGA_HPP_
#define _VGA_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"
#include "hardware.h"

// 规定显示行数、列数
static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;

// 设置颜色，详解见 '颜色设置与位运算.md'
// fg-font
// bg-back
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

// uc-字符
// color-颜色
static inline uint16_t vga_entry(uint8_t uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

#ifdef __cplusplus
}
#endif

#endif /* _VGA_HPP_ */

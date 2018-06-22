
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// vga.hpp for MRNIU/SimpleKernel.

#ifndef _VGA_HPP_
#define _VGA_HPP_

#include "stdint.h"
#include "stddef.h"
#include "port.hpp"

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define VGA_ADDR 0x3D4	// CRT 控制寄存器-地址
#define VGA_DATA 0x3D5	// CRT 控制寄存器-数据
#define VGA_CURSOR_H	0xE	// 光标高位
#define VGA_CURSOR_L	0xF	// 光标低位
#define VGA_MEM_BASE	0xB8000	// VGA 缓存基址
#define VGA_MEM_SIZE	0x8000	// VGA 缓存大小

// 规定显示行数、列数
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// 设置颜色，详解见 '颜色设置与位运算.md'
// fg-font
// bg-back
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg){
	return fg | (bg << 4);
}

// uc-字符
// color-颜色
static inline uint16_t vga_entry(unsigned char uc, uint8_t color){
	return (uint16_t) uc | (uint16_t) color << 8;
}

#endif

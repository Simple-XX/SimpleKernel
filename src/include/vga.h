
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// graphic.h for MRNIU/SimpleKernel.


#include "kernel.h"
#include "stdint.h"
#include "stddef.h"

#ifndef _VGA_H
#define _VGA_H

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

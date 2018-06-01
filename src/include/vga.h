
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// graphic.h for MRNIU/SimpleKernel.


#include "kernel.h"
#include "stdint.h"
#include "stddef.h"

#ifndef _VGA_H
#define _VGA_H

#define VGA_ADDR 0x3D4	// CRT 控制寄存器-地址
#define VGA_DATA 0x3D5	// CRT 控制寄存器-数据
#define VGA_CURSOR_H	0xE	// 光标高位
#define VGA_CURSOR_L	0xF	// 光标低位
#define VGA_MEM_BASE	0xB8000	// VGA 缓存基址
#define VGA_MEM_SIZE	0x8000	// VGA 缓存大小



// 设置颜色，详解见 '颜色设置与位运算.md'
// fg-font
// bg-back
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg){
	return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color){
	return (uint16_t) uc | (uint16_t) color << 8;
}

#endif

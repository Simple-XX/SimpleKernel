
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.h for MRNIU/SimpleKernel.

#include "stdint.h"
#include "stddef.h"
#include "stdarg.h"

#ifndef _KERNEL_H
#define _KERNEL_H


//------------------------------------------------------------------------------
// kerne.c
/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

//------------------------------------------------------------------------------
// vsprintf.c
int vsprintf(char * buf, const char * fmt, va_list args);

//------------------------------------------------------------------------------
// printk.c
int printk(const char * fmt, ...);

//------------------------------------------------------------------------------
// string.c

size_t strlen(const char* str); // 获取字符串长度

//------------------------------------------------------------------------------
// tty.h

size_t terminal_row; // 命令行行数
size_t terminal_column; // 当前命令行列数
uint8_t terminal_color; // 当前命令行颜色

// Note the use of the volatile keyword to prevent the compiler
// from eliminating dead stores.
volatile uint16_t* terminal_buffer;

// Hardware text mode color constants.


void terminal_initialize(void); // 命令行初始化

void terminal_setcolor(uint8_t color); // 设置命令行颜色

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y); // 在指定位置输出字符

void terminal_putchar(char c); // 在当前位置输出字符

void terminal_write(const char* data, size_t size); // 命令行写

void terminal_writestring(const char* data); // 命令行写字符串

void terminal_setcursor(size_t x, size_t y)	// 设置光标位置

//------------------------------------------------------------------------------
// vga.h

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

// 设置颜色，详解见 '颜色设置与位运算.md'
// fg-font
// bg-back
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
static inline uint16_t vga_entry(unsigned char uc, uint8_t color);

// 规定显示行数、列数
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

//------------------------------------------------------------------------------
// port.h

static inline void outb(uint16_t port, uint8_t value);	// 端口写一个字节
static inline uint8_t inb(uint16_t port);	// 端口读一个字节
static inline uint16_t inw(uint16_t port);	// 端口读一个字

#endif

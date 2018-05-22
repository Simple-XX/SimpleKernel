
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// tty.h for MRNIU/SimpleKernel.

#include "kernel.h"
#include "stdint.h"
#include "stddef.h"

#ifndef _TTY_H
#define _TTY_H


// 获取字符串长度
size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

// 命令行初始化
void terminal_initialize(void)
{
  // 从左上角开始
	terminal_row = 0;
	terminal_column = 0;
  // 字体为灰色，背景为黑色
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
  // 用 ' ' 填满屏幕
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

// 设置命令行颜色
void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

// 在指定位置输出字符
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}


// 转义字符处理
void terminal_escapeconv(char c){
	switch(c){
		case '\n':
			terminal_column+=1;
			break;
		case '\t':
			terminal_row+=4;
			break;
	}
}

// 在当前位置输出字符
void terminal_putchar(char c)
{
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	// 转义字符处理
	terminal_escapeconv(c);
	// 如果到达最后一列则换行
  if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
    // 如果到达最后一行则从第一行重新开始
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

// 命令行写
void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}


// 命令行写字符串
void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}



#endif

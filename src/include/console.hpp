
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// tty.hpp for MRNIU/SimpleKernel.

#ifndef _CONSOLE_HPP_
#define _CONSOLE_HPP_

#include "stdint.h"
#include "stddef.h"
#include "vga.hpp"
#include "string.h"

size_t console_row; // 命令行行数
size_t console_column; // 当前命令行列数
uint8_t console_color; // 当前命令行颜色

static uint16_t * console_buffer = (uint16_t*) VGA_MEM_BASE;

void console_scroll(void);
uint16_t console_getcursorpos(void);
void console_setcursorpos(size_t, size_t);
void console_writestring(const char*);
void console_write(const char*, size_t);
void console_putchar(char);
void console_escapeconv(char);
void console_putentryat(char, uint8_t, size_t, size_t);
void console_setcolor(uint8_t);
uint8_t console_getcolor(void);


// 命令行初始化
void console_init(void);
void console_init(void){
		// 从左上角开始
		console_row = 0;
		console_column = 0;
		// 字体为灰色，背景为黑色
		console_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
		// 用 ' ' 填满屏幕
		for (size_t y = 0; y < VGA_HEIGHT; y++) {
				for (size_t x = 0; x < VGA_WIDTH; x++) {
						const size_t index = y * VGA_WIDTH + x;
						console_buffer[index] = vga_entry(NULL, console_color);
				}
		}
		console_setcursorpos(0, 0);
}

// 设置命令行颜色
void console_setcolor(uint8_t color){
		console_color = color;
}

// 获取当前颜色
uint8_t console_getcolor(void){
		return console_color;
}

// 在指定位置输出字符
void console_putentryat(char c, uint8_t color, size_t x, size_t y){
		const size_t index = y * VGA_WIDTH + x;
		console_buffer[index] = vga_entry(c, color);
}

// 转义字符处理
void console_escapeconv(char c){
		switch(c) {
		case '\n':
				console_column=0;
				console_row++;
				break;
		case '\t':
				// 取整对齐
				console_column=(console_column+8)&~(8-1);
				// 如果到达最后一列则换行
				if (++console_column >= VGA_WIDTH) {
						console_column=0;
						console_row++;
				}
				break;
		case '\b':
				if(console_column)
						console_column-=2;
				break;
		}
}

// 在当前位置输出字符
void console_putchar(char c){
		console_putentryat(c, console_color, console_column, console_row);
		// 如果到达最后一列则换行
		if (++console_column >= VGA_WIDTH) {
				console_column=0;
				console_row++;
		}
		console_escapeconv(c); // 转义字符处理
		console_scroll(); // 屏幕滚动
		console_setcursorpos(console_column, console_row);
}

// 命令行写
void console_write(const char* data, size_t size){
		for (size_t i = 0; i < size; i++)
				console_putchar(data[i]);
}

// 命令行写字符串
void console_writestring(const char* data){
		console_write(data, strlen(data));
}

// 设置光标位置
void console_setcursorpos(size_t x, size_t y){
		const uint16_t index = y * VGA_WIDTH + x;
		// 光标的设置，见参考资料
		outb(VGA_ADDR, VGA_CURSOR_H);   // 告诉 VGA 我们要设置光标的高字节
		outb(VGA_DATA, index >> 8);   // 发送高 8 位
		outb(VGA_ADDR, VGA_CURSOR_L);   // 告诉 VGA 我们要设置光标的低字节
		outb(VGA_DATA, index);   // 发送低 8 位
}

// 获取光标位置
uint16_t console_getcursorpos(){
		outb(VGA_ADDR, VGA_CURSOR_H);
		size_t cursor_pos_h = inb(VGA_DATA);
		outb(VGA_ADDR, VGA_CURSOR_L);
		size_t cursor_pos_l = inb(VGA_DATA);
		//返回光标位置
		return (cursor_pos_h << 8) | cursor_pos_l;
}

// 滚动显示
void console_scroll(){
		if (console_row >= VGA_HEIGHT) {
				// 将所有行的显示数据复制到上一行
				for (size_t i= 0; i < (VGA_HEIGHT-1)* VGA_WIDTH; i++)
						console_buffer[i] = console_buffer[i+VGA_WIDTH];
				// 最后的一行数据现在填充空格，不显示任何字符
				for (size_t i= (VGA_HEIGHT-1)* VGA_WIDTH; i< VGA_HEIGHT* VGA_WIDTH; i++)
						console_buffer[i] = vga_entry(' ', console_color);
				// 向上移动了一行，所以 cursor_y 现在是 24
				console_row = 24;
		}
}


#endif

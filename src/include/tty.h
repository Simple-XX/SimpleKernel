
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// tty.h for MRNIU/SimpleKernel.

#include "kernel.h"
#include "stdint.h"
#include "stddef.h"

#ifndef _TTY_H
#define _TTY_H

// 命令行初始化
void terminal_init(void){
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
	terminal_setcursorpos(0, 0);
}

// 设置命令行颜色
void terminal_setcolor(uint8_t color){
	terminal_color = color;
}

// 在指定位置输出字符
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y){
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

// 转义字符处理
void terminal_escapeconv(char c){
	switch(c){
		case '\n':
			terminal_row+=1;
			terminal_column=0;
			break;
		case '\t':
			terminal_column+=4;
			break;
		case '\b':
			terminal_column-=1;
			break;
	}
}

// 在当前位置输出字符
void terminal_putchar(char c){
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

	// 如果到达最后一列则换行
  if (++terminal_column == VGA_WIDTH)
		terminal_row += 1;
	terminal_escapeconv(c);	// 转义字符处理
	terminal_scroll(); // 屏幕滚动
	terminal_setcursorpos(terminal_column, terminal_row);
}

// 命令行写
void terminal_write(const char* data, size_t size){
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

// 命令行写字符串
void terminal_writestring(const char* data){
	terminal_write(data, strlen(data));
}

// 设置光标位置
void terminal_setcursorpos(size_t x, size_t y){
    const uint16_t index = y * VGA_WIDTH + x;
    // 光标的设置，见参考资料
    outb(VGA_ADDR, VGA_CURSOR_H);	// 告诉 VGA 我们要设置光标的高字节
    outb(VGA_DATA, index >> 8);	// 发送高 8 位
    outb(VGA_ADDR, VGA_CURSOR_L);	// 告诉 VGA 我们要设置光标的低字节
    outb(VGA_DATA, index);	// 发送低 8 位
}

// 获取光标位置
uint16_t terminal_getcursorpos(){
	outb(VGA_ADDR, VGA_CURSOR_H);
	size_t cursor_pos_h = inb(VGA_DATA);
	outb(VGA_ADDR, VGA_CURSOR_L);
	size_t cursor_pos_l = inb(VGA_DATA);
  //返回光标位置
  return (cursor_pos_h << 8) | cursor_pos_l;
}

// 滚动显示
void terminal_scroll(){
	if (terminal_row >= VGA_HEIGHT) {
		// 将所有行的显示数据复制到上一行
		for (size_t i= 0; i < (VGA_HEIGHT-1)* VGA_WIDTH; i++)
			terminal_buffer[i] = terminal_buffer[i+80];
		// 最后的一行数据现在填充空格，不显示任何字符
		for (size_t i= (VGA_HEIGHT-1)* VGA_WIDTH; i< VGA_HEIGHT* VGA_WIDTH; i++)
			terminal_buffer[i] = vga_entry(' ', terminal_color);
		// 向上移动了一行，所以 cursor_y 现在是 24
		terminal_row = 24;
  }
}


#endif

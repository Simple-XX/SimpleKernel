
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on Orange's 一个操作系统的实现
// keyboard.h for MRNIU/SimpleKernel.

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 键盘缓冲区大小
#define KB_BUFSIZE 128

#define KB_DATA 0x60
#define KB_WRITE 0x60
#define KB_STATUS 0x64
#define KB_CMD 0x64

// 特殊键的扫描码
#define KB_BACKSPACE 0x0E
#define KB_ENTER 0x1C
#define KB_TAB 0x0F
#define KB_ESC 0x01
#define KB_SHIFT_L 0x2A
#define KB_SHIFT_R 0x36
#define KB_ALT_L 0x38
#define KB_CAPS_LOCK 0x3A
#define KB_CTRL_L 0x1D
#define KB_NUM_LOCK 0x45

#define MAP_COLS 3  // Number of columns in keyma
#define NR_SCAN_CODES 0x80  // Number of scan codes (rows in keymap)
#define SC_MAX NR_SCAN_CODES * MAP_COLS

// 8 位的键盘扫描码的接通码使用前7位
// 其最高位置 1 即是其对应的断开码
// 该宏可以和获取的扫描码用来判断一个键是按下还是抬起
#define RELEASED_MASK 0x80

#define FLAG_EXT 0x00  // Normal function keys

// Special keys
#define ESC  (0x01 & FLAG_EXT) // Esc
#define TAB  (0x02 & FLAG_EXT) // Tab
#define ENTER  (0x03 & FLAG_EXT) // Enter
#define BACKSPACE (0x04 & FLAG_EXT) // BackSpace

#define GUI_L  (0x05 & FLAG_EXT) // L GUI
#define GUI_R  (0x06 & FLAG_EXT) // R GUI
#define APPS  (0x07 & FLAG_EXT) // APPS

// Shift, Ctrl, Alt
#define SHIFT_L  (0x08 & FLAG_EXT) // L Shift
#define SHIFT_R  (0x09 & FLAG_EXT) // R Shift
#define CTRL_L  (0x0A & FLAG_EXT) // L Ctrl
#define CTRL_R  (0x0B & FLAG_EXT) // R Ctrl
#define ALT_L  (0x0C & FLAG_EXT) // L Alt
#define ALT_R  (0x0D & FLAG_EXT) // R Alt

// Lock keys
#define CAPS_LOCK (0x0E & FLAG_EXT) // Caps Lock
#define NUM_LOCK (0x0F & FLAG_EXT) // Number Lock
#define SCROLL_LOCK (0x10 & FLAG_EXT) // Scroll Lock

// Function keys
#define F1  (0x11 & FLAG_EXT) // F1
#define F2  (0x12 & FLAG_EXT) // F2
#define F3  (0x13 & FLAG_EXT) // F3
#define F4  (0x14 & FLAG_EXT) // F4
#define F5  (0x15 & FLAG_EXT) // F5
#define F6  (0x16 & FLAG_EXT) // F6
#define F7  (0x17 & FLAG_EXT) // F7
#define F8  (0x18 & FLAG_EXT) // F8
#define F9  (0x19 & FLAG_EXT) // F9
#define F10  (0x1A & FLAG_EXT) // F10
#define F11  (0x1B & FLAG_EXT) // F11
#define F12  (0x1C & FLAG_EXT) // F12

// Control Pad
#define PRINTSCREEN (0x1D & FLAG_EXT) // Print Screen
#define PAUSEBREAK (0x1E & FLAG_EXT) // Pause/Break
#define INSERT  (0x1F & FLAG_EXT) // Insert
#define DELETE  (0x20 & FLAG_EXT) // Delete
#define HOME  (0x21 & FLAG_EXT) // Home
#define END  (0x22 & FLAG_EXT) // End
#define PAGEUP  (0x23 & FLAG_EXT) // Page Up
#define PAGEDOWN (0x24 & FLAG_EXT) // Page Down
#define UP  (0x25 & FLAG_EXT) // Up
#define DOWN  (0x26 & FLAG_EXT) // Down
#define LEFT  (0x27 & FLAG_EXT) // Left
#define RIGHT  (0x28 & FLAG_EXT) // Right

// ACPI keys
#define POWER  (0x29 & FLAG_EXT) // Power
#define SLEEP  (0x2A & FLAG_EXT) // Sleep
#define WAKE  (0x2B & FLAG_EXT) // Wake Up

// Num Pad
#define PAD_SLASH (0x2C & FLAG_EXT) // /
#define PAD_STAR (0x2D & FLAG_EXT) // *
#define PAD_MINUS (0x2E & FLAG_EXT) // -
#define PAD_PLUS (0x2F & FLAG_EXT) // &
#define PAD_ENTER (0x30 & FLAG_EXT) // Enter
#define PAD_DOT  (0x31 & FLAG_EXT) // .
#define PAD_0  (0x32 & FLAG_EXT) // 0
#define PAD_1  (0x33 & FLAG_EXT) // 1
#define PAD_2  (0x34 & FLAG_EXT) // 2
#define PAD_3  (0x35 & FLAG_EXT) // 3
#define PAD_4  (0x36 & FLAG_EXT) // 4
#define PAD_5  (0x37 & FLAG_EXT) // 5
#define PAD_6  (0x38 & FLAG_EXT) // 6
#define PAD_7  (0x39 & FLAG_EXT) // 7
#define PAD_8  (0x3A & FLAG_EXT) // 8
#define PAD_9  (0x3B & FLAG_EXT) // 9
#define PAD_UP  PAD_8  // Up
#define PAD_DOWN PAD_2  // Down
#define PAD_LEFT PAD_4  // Left
#define PAD_RIGHT PAD_6  // Right
#define PAD_HOME PAD_7  // Home
#define PAD_END  PAD_1  // End
#define PAD_PAGEUP PAD_9  // Page Up
#define PAD_PAGEDOWN PAD_3  // Page Down
#define PAD_INS  PAD_0  // Ins
#define PAD_MID  PAD_5  // Middle key
#define PAD_DEL  PAD_DOT  // Del

void keyboard_init(void);
void keyboard_handler(void);
void keyboard_read(pt_regs_t * regs);
void keyboard_read(pt_regs_t * regs);
uint8_t keyboard_read_from_buff(void);

typedef
    struct kb_input {
	uint8_t *	head;
	uint8_t *	tail;
	size_t		count;
	uint8_t		buff[KB_BUFSIZE];
} kb_input_t;

#ifdef __cplusplus
}
#endif

#endif /* _KEYBOARD_H_ */

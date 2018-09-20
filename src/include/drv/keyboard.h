
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// keyboard.h for MRNIU/SimpleKernel.

#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include "stdint.h"
#include "intr/intr.h"

// 键盘缓冲区大小
#define KB_BUFSIZE 50
#define KB_DATA 0x60
#define KB_WRITE 0x60
#define KB_STATUS 0x64
#define KB_CMD 0x64
#define KB_READ 0x20
#define KB_INIT_MODE 0x47
#define BACKSPACE 0x0E
#define KB_ENTER 0x1C

enum KB_CTRL_STATS_MASK {
	KB_CTRL_STATS_MASK_OUT_BUF	=	1,		//00000001
	KB_CTRL_STATS_MASK_IN_BUF	=	2,		//00000010
	KB_CTRL_STATS_MASK_SYSTEM	=	4,		//00000100
	KB_CTRL_STATS_MASK_CMD_DATA	=	8,		//00001000
	KB_CTRL_STATS_MASK_LOCKED	=	0x10,		//00010000
	KB_CTRL_STATS_MASK_AUX_BUF	=	0x20,		//00100000
	KB_CTRL_STATS_MASK_TIMEOUT	=	0x40,		//01000000
	KB_CTRL_STATS_MASK_PARITY	=	0x80		//10000000
};

// 等待输入缓冲区满
#define wait_to_write() while(inb(KB_STATUS) & KB_CTRL_STATS_MASK_IN_BUF)
// 等待输出缓冲区满
#define wait_to_read() while(inb(KB_STATUS) & KB_CTRL_STATS_MASK_OUT_BUF)


// 键盘缓冲区结构
typedef
struct kb_buffer_t{
  uint8_t * head;
  uint8_t * tail;
  uint32_t count;
  uint8_t buf[KB_BUFSIZE];
} kb_buffer_t;

extern void init_interrupt_chip(void);
extern void clear_interrupt_chip(uint32_t intr_no); // 重置 8259A
void keyboard_init(void);
void keyboard_handler(pt_regs_t * regs);


#endif

// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kb.c for MRNIU/SimpleKernel.

#include "drv/keyboard.h"
#include "stdio.h"
#include "stddef.h"
#include "port.hpp"


static unsigned char keyboard_map[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};


void keyboard_init(void){
  printk("kb init\n");
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	outb(0x21 , 0xFD);
  register_interrupt_handler(IRQ0, &keyboard_handler);
}

void keyboard_handler(pt_regs_t * regs){
  unsigned char status;
	char keycode;
	/* write EOI */
	outb(0x20, 0x20);
	status = inb(KB_STATUS);
	/* Lowest bit of status will be set if buffer is not empty */
  if(status!=0){
    printk("%d", status);
  }
	if (status & 0x01) {
    keycode = inb(KB_DATA);
		if(keycode < 0)
      printk("eooreddsd");
			return;
		if(keycode == KB_ENTER) {
      printk("\n");
			return;
		}
    printk("%c", keyboard_map[(unsigned char) keycode]);
		//vidptr[current_loc++] = 0x07;
	}
  UNUSED(regs);
}

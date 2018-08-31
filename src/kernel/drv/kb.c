// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kb.c for MRNIU/SimpleKernel.

#include "drv/keyboard.h"
#include "stdio.h"
#include "stddef.h"
#include "port.hpp"


void kb_open_A20(void){
  outb(KB_CMD, 0xd1);
  outb(KB_WRITE, 0xdf);
}

void kb_reboot(void){
  outb(KB_CMD, 0xfe);
}

#define SC_MAX 57
const char *sc_name[] = { "ERROR", "Esc", "1", "2", "3", "4", "5", "6",
    "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
        "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
        "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
        "/", "RShift", "Keypad *", "LAlt", "Spacebar"};

const char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
        'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G',
        'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};
static char kb_buffer[16];

void keyboard_callback(pt_regs_t * regs){
  printk("233");
  uint8_t sccode =inb(KB_DATA);
  if(sccode>SC_MAX)
    printk_color(red, "Keyboard Error!");
  if(sccode==BACKSPACE){
    backspace(kb_buffer);
  } else if(sccode==ENTER){
    printk("\n");
    kb_buffer[0]='\0';
  } else{
    char letter=sc_ascii[(int)sccode];
    char str[2]={letter, '\0'};
    append(kb_buffer, letter);
    printk("%s", str);
  }
  UNUSED(regs);
}
/*
void test(pt_regs_t * regs){
  unsigned char x;
  x=inb(0x60);
  printk("%c\n", x);
  outb(0x20, 0x20);
  UNUSED(regs);
}

void keyboard_init(void){
  printk("initialize keyboard\n");
  //register_interrupt_handler(IRQ1, &keyboard_callback);
  register_interrupt_handler(IRQ1, &test);
}
*/

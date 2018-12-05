// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kb.c for MRNIU/SimpleKernel.

#include "drv/keyboard.h"
#include "port.hpp"


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

#define SC_MAX 57
static char key_buffer[256];
void keyboard_handler(pt_regs_t * regs){
  uint8_t scancode = inb(KB_DATA);
  if(scancode>SC_MAX)
    return;
  if(scancode == KB_BACKSPACE)
    backspace(key_buffer);
  else if(scancode == KB_ENTER){
    printk("\n");
    key_buffer[0] = '\0';
  } else{
    char letter = sc_ascii[(int)scancode];
    char str[2]={letter,'\0'};
    append(key_buffer, letter);
    printk("%s\n", str);
  }
  UNUSED(regs);
}

void keyboard_init(void){
  printk("kb init\n");
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	outb(0x21 , 0xFD);
  register_interrupt_handler(IRQ1, &keyboard_handler);
}

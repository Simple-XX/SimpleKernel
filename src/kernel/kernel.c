
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.


#include "../include/kernel.h"
#include "../include/vga.h"
#include "../include/tty.h"
#include "../include/port.h"



void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();
	gdt_init();

	printk("Welcome to my kernel.\n");
	printk("terminal_color: %d\n", terminal_color);
	printk("current cursor pos: %d\n", terminal_getcursorpos());


}

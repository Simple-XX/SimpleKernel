
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


	//printk("terminal_color: %d\n", vga_color[terminal_color]);
	//printk("%d", terminal_getcursorpos());
	for(int i=0;i<30;i++)
		printk("%dwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww", i);




}

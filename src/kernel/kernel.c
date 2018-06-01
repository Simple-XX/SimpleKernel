
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

	/* Newline support is left as an exercise. */
	printk("Hello Kernel!\n");


}

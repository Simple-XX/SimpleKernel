
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

//#include <stdbool.h>
#include "../include/kernel.h"
#include "../include/vga.h"
#include "../include/tty.h"

void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();

	/* Newline support is left as an exercise. */
	int a='!';
	char c='!';
	char * s="gg";
	printk("__%c__%s",c,s);


}

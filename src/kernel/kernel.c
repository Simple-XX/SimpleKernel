
// This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

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
	int a=233;
	char c='!';
	char * s="gg";
	printk("__%d__%s__%c",a,s,c);
	printk("\n");
	printk("_%c_%d_%s\n/n",c,a,s);
	printk("end");


}

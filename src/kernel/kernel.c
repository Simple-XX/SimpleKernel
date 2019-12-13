
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/kernel.h"

void kernel_main(void) {
	console_init(); // 控制台初始化

	char c='!';
	char * s="gg";
	printk("__%c__%s\n",c,s);
	printk("Simple Kernel\n");
	return;
}

#ifdef __cplusplus
}
#endif

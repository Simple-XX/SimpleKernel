
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.h for MRNIU/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined( __linux__ )
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#include "stdint.h"
#include "stdio.h"

void kernel_main(void);
void console_init(void);

#endif /* _KERNEL_H_ */

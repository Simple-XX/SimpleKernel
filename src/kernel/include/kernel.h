
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.h for MRNIU/SimpleKernel.


#ifndef _KERNEL_H_
#define _KERNEL_H_

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined( __linux__ )
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
// #if !defined( __i386__ ) | !defined(__x86_64__)
// #error "Please compile that with a ix86-elf compiler"
// #endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

void kernel_main(uint32_t magic, uint32_t addr);

#endif /* _KERNEL_H_ */

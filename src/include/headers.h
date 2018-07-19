
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.h for MRNIU/SimpleKernel.


#ifndef _KERNEL_H_
#define _KERNEL_H_

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

#include "stdio.h"
#include "tty.hpp"
#include "multiboot.h"
#include "elf.h"
#include "debug.h"
#include "mm/pmm.h"
#include "mm/mm.h"

void debug_init(void);
void gdt_init(void);
void idt_init(void);



#endif


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
// #include "../drv/console/include/tty.h"
// #include "../drv/console/include/console.h"
// #include "include/debug.h"
// #include "mm/include/pmm.h"
// #include "mm/include/vmm.h"
// #include "../drv/clock/include/clock.h"
// #include "cpu.hpp"
// #include "../drv/keyboard/include/keyboard.h"
// #include "include/elf.h"
// #include "include/heap.h"
//
//
// void kernel_main(uint32_t magic, uint32_t addr);
// void console_init(void);
// void gdt_init(void);
// void idt_init(void);
// void clock_init(void);
// void keyboard_init(void);
// void mouse_init(void);
// void debug_init(uint32_t magic, uint32_t addr);
// void pmm_init(multiboot_tag_t * tag);
// void vmm_init();

#endif /* _KERNEL_H_ */

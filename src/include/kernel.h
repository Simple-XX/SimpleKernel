
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

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "tty.hpp"
#include "multiboot.h"
#include "elf.h"
#include "debug.h"
#include "mm/pmm.h"
#include "mm/mm.h"
#include "clock.h"
#include "cpu.hpp"
#include "drv/keyboard.h"

void debug_init(void);
void gdt_init(void);
void idt_init(void);
void clock_init(void);

void issti(void){
  cpu_sti();
  if(canint())
    printk_color(white, "interrupt accept!\n");
  else
    printk_color(light_red, "interrupt closed!\n");
}

void showinfo(void){
  // 输出一些基本信息
  printk_color(magenta ,"Welcome to my kernel.\n");
  printk_color(light_red ,"kernel in memory start: 0x%08X\n", kern_start);
  printk_color(light_red ,"kernel in memory end: 0x%08X\n", kern_end);
  printk_color(light_red ,"kernel in memory size: %d KB, %d pages\n",
              (kern_end - kern_start) / 1024, (kern_end - kern_start) / 1024 / 4);
  issti();
}

#endif

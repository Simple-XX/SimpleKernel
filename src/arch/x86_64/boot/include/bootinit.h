
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.h for MRNIU/SimpleKernel.

#ifndef _BOOTINIT_H_
#define _BOOTINIT_H_

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
#include "mem/vmm.h"

// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0-640KB 肯定是空闲的
__attribute__( ( section(".init.data") ) ) pgd_t *pgd_tmp  = (pgd_t *)0x1000;
__attribute__( ( section(".init.data") ) ) pte_t *pte_low  = (pte_t *)0x2000;
__attribute__( ( section(".init.data") ) ) pte_t *pte_high = (pte_t *)0x3000;

__attribute__( ( section(".init.text") ) ) void kernel_entry(uint32_t magic, uint32_t addr);
__attribute__( ( section(".init.text") ) ) void enable_page(uint32_t * pgd);
__attribute__( ( section(".init.text") ) ) void mm_init(void);
extern void kernel_main(uint32_t magic, uint32_t addr);


#endif /* _BOOTINIT_H_ */

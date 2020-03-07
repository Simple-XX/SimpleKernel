
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.h for MRNIU/SimpleKernel.

#ifndef _BOOTINIT_H_
#define _BOOTINIT_H_

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined (__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
// #if !defined( __i386__ ) | !defined(__x86_64__)
// #error "Please compile that with a ix86-elf compiler"
// #endif

#include "stdint.h"

// 内核使用的临时页表和页目录
// 用于映射 init 段与内核段
__attribute__( (section(".init.data") ) )
pgd_t pgd_tmp[VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY] __attribute__( (aligned(VMM_PAGE_SIZE) ) );
// init 段
__attribute__( (section(".init.data") ) )
pte_t pte_init[VMM_PAGES_PRE_PAGE_TABLE] __attribute__( (aligned(VMM_PAGE_SIZE) ) );
// 内核段，共 8MB
__attribute__( (section(".init.data") ) )
pte_t pte_kernel_tmp[VMM_PAGES_PRE_PAGE_TABLE] __attribute__( (aligned(VMM_PAGE_SIZE) ) );
__attribute__( (section(".init.data") ) )
pte_t pte_kernel_tmp2[VMM_PAGES_PRE_PAGE_TABLE] __attribute__( (aligned(VMM_PAGE_SIZE) ) );
// 内核栈区域
__attribute__( (section(".init.data") ) )
pte_t pte_kernel_stack_tmp[VMM_PAGES_PRE_PAGE_TABLE] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

__attribute__( (section(".init.text") ) ) void kernel_entry(ptr_t magic, ptr_t addr);
__attribute__( (section(".init.text") ) ) void enable_page(pgd_t * pgd);
__attribute__( (section(".init.text") ) ) void mm_init(void);

extern void kernel_main(ptr_t magic, ptr_t addr);

#ifdef __cplusplus
}
#endif

#endif /* _BOOTINIT_H_ */

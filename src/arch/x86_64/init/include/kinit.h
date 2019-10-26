
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// init.h for MRNIU/SimpleKernel.

#ifndef _INIT_H_
#define _INIT_H_

#include "stdint.h"
#include "mm/include/vmm.h"
#include "gdt.h"


// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0-640KB 肯定是空闲的
__attribute__( ( section(".init.data") ) ) pgd_t *pgd_tmp  = (pgd_t *)0x1000;
__attribute__( ( section(".init.data") ) ) pte_t *pte_low  = (pte_t *)0x2000;
__attribute__( ( section(".init.data") ) ) pte_t *pte_high = (pte_t *)0x3000;



__attribute__( ( section(".init.text") ) ) void kernel_entry(uint32_t magic, uint32_t addr);

extern void kernel_main(uint32_t magic, uint32_t addr);


#endif /* _INIT_H_ */

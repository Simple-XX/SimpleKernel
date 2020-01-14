
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "mem/vmm.h"
#include "mem/pmm.h"
#include "debug.h"
#include "stdio.h"
#include "string.h"
#include "cpu.hpp"

// 内核页目录区域
static pgd_t pgd_kernel[VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

// 内核页表区域
static pte_t pte_kernel[VMM_PAGES_PRE_PAGE_TABLE] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

// 内核页表起始
// static pte_t * pte_addr = (pte_t *)( (uint32_t)kern_end + KERNBASE);
// 页表数组指针
// pte_t(*pte_kern)[PTE_SIZE] = (pte_t(*)[PTE_SIZE])pte_addr;


void vmm_init(void) {
	register_interrupt_handler(INT_PAGE_FAULT, &page_fault);

	// 内核段 pgd_tmp[0x300], 4MB
	// pgd_kernel[VMM_PGD_INDEX(KERNEL_BASE)] = ( (uint32_t)pte_kernel - KERNEL_BASE) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	uint32_t pgd_idx = VMM_PGD_INDEX(KERNEL_BASE);
	for(uint32_t i = pgd_idx  ; i < VMM_PAGE_DIRECTORIES_KERNEL + pgd_idx ; i++) {
		pgd_kernel[i] = (vmm_la_to_pa( (ptr_t)pte_kernel) | VMM_PAGE_PRESENT | VMM_PAGE_RW);
	}

	// 将每个页表项赋值
	// 映射 kernel 段 4MB
	// 映射 0x00000000-0x00400000 的物理地址到虚拟地址 0xC0000000-0xC0400000
	// pgd_tmp[0x300] => pte_kernel
	for(uint32_t i = 0 ; i < 1024 ; i++) {
		pte_kernel[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	}

	// 首先映射内核页目录
	// ptr_t pgd_idx = VMM_PGD_INDEX(KERNEL_BASE);
	// for(uint32_t i = pgd_idx, j = 0 ; i < VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY + pgd_idx ; i++, j++) {
	// 	pgd_kernel[i] = (pgd_t)vmm_la_to_pa( (ptr_t)pte_kernel[j]) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	// }
	// // 映射所有页到页表中
	// printk_test("------111111\n");
	// // ptr_t * tmp = (ptr_t *)(pte_kernel + VMM_PAGES_PRE_PAGE_TABLE * pgd_idx);
	// ptr_t * tmp = (ptr_t *)pte_kernel;
	// printk_test("090000\n");
	// for(uint32_t i = 0 ; i < VMM_PAGES_TOTAL ; i++) {
	// 	tmp[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	// }

	printk_test("111111\n");
	switch_pgd( (ptr_t)pgd_kernel - KERNEL_BASE);
	printk_test("22222\n");

	printk_info("vmm_init\n");

	return;
}

void map(pgd_t * pgd_now, ptr_t va, ptr_t pa, uint32_t flags) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);

	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	if(!pte) {
		pte = (pte_t *)pmm_alloc(1);
		pgd_now[pgd_idx] = (ptr_t)pte | VMM_PAGE_PRESENT | VMM_PAGE_RW;

		// 转换到内核线性地址并清 0
		pte = (pte_t *)( (ptr_t)pte + KERNEL_BASE);
		bzero(pte, VMM_PAGE_SIZE);
	} else {
		// 转换到内核线性地址
		pte = (pte_t *)( (ptr_t)pte + KERNEL_BASE);
	}

	pte[pte_idx] = (pa & VMM_PAGE_MASK) | flags;

	// 通知 CPU 更新页表缓存
	__asm__ volatile ("invlpg (%0)" : : "a" (va) );
	// __native_flush_tlb_single(va);
}

void unmap(pgd_t * pgd_now, ptr_t va) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);

	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);

	if(!pte) {
		return;
	}

	// 转换到内核线性地址
	pte = (pte_t *)( (ptr_t)pte + KERNEL_BASE);

	pte[pte_idx] = 0;

	// 通知 CPU 更新页表缓存
	__asm__ volatile ("invlpg (%0)" : : "a" (va) );
	// __native_flush_tlb_single(va);
}

uint32_t get_mapping(pgd_t * pgd_now, ptr_t va, ptr_t pa) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);

	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	if(!pte) {
		return 0;
	}

	// 转换到内核线性地址
	pte = (pte_t *)( (ptr_t)pte + KERNEL_BASE);

	// 如果地址有效而且指针不为NULL，则返回地址
	if(pte[pte_idx] != 0 && pa) {
		pa = pte[pte_idx] & VMM_PAGE_MASK;
		return 1;
	}

	return 0;
}

void switch_pgd(ptr_t pd) {
	__asm__ volatile ("mov %0, %%cr3" : : "r" (pd) );
}

void page_fault(pt_regs_t * regs) {
#ifdef __x86_64__
	uint64_t cr2;
	asm volatile ("movq %%cr2,%0" : "=r" (cr2) );
#else
	uint32_t cr2;
	asm volatile ("mov %%cr2,%0" : "=r" (cr2) );
#endif
	printk("Page fault at 0x%08X, virtual faulting address 0x%08X\n", regs->eip, cr2);
	printk_err("Error code: 0x%08X\n", regs->err_code);

	// bit 0 为 0 指页面不存在内存里
	if(!(regs->err_code & 0x1) )
		printk_color(red, "Because the page wasn't present.\n");
	// bit 1 为 0 表示读错误，为 1 为写错误
	if(regs->err_code & 0x2)
		printk_err("Write error.\n");
	else
		printk_err("Read error.\n");
	// bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
	if(regs->err_code & 0x4)
		printk_err("In user mode.\n");
	else
		printk_err("In kernel mode.\n");
	// bit 3 为 1 表示错误是由保留位覆盖造成的
	if(regs->err_code & 0x8)
		printk_err("Reserved bits being overwritten.\n");
	// bit 4 为 1 表示错误发生在取指令的时候
	if(regs->err_code & 0x10)
		printk_err("The fault occurred during an instruction fetch.\n");
	while(1);
}

#ifdef __cplusplus
}
#endif

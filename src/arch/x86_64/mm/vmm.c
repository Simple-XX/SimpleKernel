
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
pgd_t pgd_kernel[VMM_PAGE_TABLES_PRE_PAGE_DIRECTORY] __attribute__( (aligned(VMM_PAGE_SIZE) ) );
// 内核页表区域
static pte_t pte_kernel[VMM_PAGE_TABLES_KERNEL][VMM_PAGES_PRE_PAGE_TABLE] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

void vmm_init(void) {
	register_interrupt_handler(INT_PAGE_FAULT, &page_fault);

	// 内核段 pgd_tmp[0x300], 4MB
	uint32_t pgd_idx = VMM_PGD_INDEX(KERNEL_BASE);
	for(uint32_t i = pgd_idx, j = 0 ; i < VMM_PAGE_DIRECTORIES_KERNEL + pgd_idx ; i++, j++) {
		pgd_kernel[i] = ( (ptr_t)vmm_la_to_pa( (ptr_t)pte_kernel[j]) | VMM_PAGE_PRESENT | VMM_PAGE_RW);
	}

	// 将每个页表项赋值
	// 映射 kernel 段 4MB
	// 映射虚拟地址 0xC0000000-0xC0400000 到物理地址 0x00000000-0x00400000
	ptr_t * pte = (ptr_t *)pte_kernel;
	for(uint32_t i = 0 ; i < VMM_PAGE_TABLES_KERNEL * VMM_PAGES_PRE_PAGE_TABLE ; i++) {
		// 物理地址由 (i << 12) 给出
		pte[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	}

	switch_pgd(vmm_la_to_pa( (ptr_t)pgd_kernel) );

	printk_info("vmm_init\n");

	return;
}

void map(pgd_t * pgd_now, ptr_t va, ptr_t pa, uint32_t flags) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);// 0x200
	uint32_t pte_idx = VMM_PTE_INDEX(va);// 0x0
	printk_debug("pgd_now = %X\n", pgd_now);
	printk_debug("pgd_idx = %X\n", pgd_idx);
	printk_debug("pte_idx = %X\n", pte_idx);
	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	printk_debug("pte1 = %X\n", pte);
	// 转换到内核线性地址
	if(pte != NULL) {
		printk_debug("pte20 = %X\n", pte);
		pte = (pte_t *)vmm_pa_to_la( (ptr_t)pte);
		printk_debug("pte21 = %X\n", pte);
	} else {
		pte = (pte_t *)vmm_pa_to_la( (ptr_t)pte);
		printk_debug("pte2 = %X\n", pte);
		pgd_now[pgd_idx] = (uint32_t)pte | flags;
		printk_debug("pgd_now[pgd_idx] = %X\n", pgd_now[pgd_idx]);
	}

	pte[pte_idx] = (pa & VMM_PAGE_MASK) | flags;
	printk_debug("pte[pte_idx] = %X\n", pte[pte_idx]);
	// // 通知 CPU 更新页表缓存
	CPU_INVLPG(va);
	return;
}

void unmap(pgd_t * pgd_now, ptr_t va) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);
	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	// 转换到内核线性地址
	pte = (pte_t *)vmm_pa_to_la( (ptr_t)pte);
	pte[pte_idx] = 0;
	// 通知 CPU 更新页表缓存
	CPU_INVLPG(va);
	return;
}

uint32_t get_mapping(pgd_t * pgd_now, ptr_t va, ptr_t pa) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);

	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	if(!pte) {
		return 0;
	}
	// 转换到内核线性地址
	pte = (pte_t *)vmm_pa_to_la( (ptr_t)pte);

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
	__asm__ volatile ("movq %%cr2,%0" : "=r" (cr2) );
#else
	uint32_t cr2;
	__asm__ volatile ("mov %%cr2,%0" : "=r" (cr2) );
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


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
static pgd_t pgd_kernel[VMM_PGD_COUNT] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

// 内核页表区域
static pte_t pte_kernel[VMM_PTE_COUNT] __attribute__( (aligned(VMM_PAGE_SIZE) ) );

void vmm_init(void) {
	// TODO
	// 使用 vmm.h 中的结构体重写该部分

	// 首先需要这么几个东西
	// 页目录，页表，页表项
	// 页目录用于保存页表地址
	// 页表每个项对应一个页表
	// 页表项保存了相关属性信息，见 page 结构体


	// 页表数组指针
	// pte_t (*pte_kern)[PTE_SIZE] = (pte_t (*)[PTE_SIZE])pte_addr;
	//
	// // 构造页目录(MMU需要的是物理地址，此处需要减去偏移)
	// uint32_t pgd_idx = PGD_INDEX(PAGE_OFFSET);
	// for (uint32_t i = pgd_idx; i < pgd_idx + PTE_COUNT; ++i) {
	// 	pgd_kern[i] = ((uint32_t)ka_to_pa(pte_kern[i])) | PAGE_PRESENT | PAGE_WRITE;
	// }
	//
	// // 构造页表映射，内核 0xC0000000～0xF8000000 映射到 物理 0x00000000～0x38000000 (物理内存前896MB)
	// uint32_t *pte_start = (uint32_t *)(pte_addr + PTE_SIZE * pgd_idx);
	// for (uint32_t i = 0; i < PTE_SIZE * PTE_COUNT; ++i) {
	// 	pte_start[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	// }
	//

	for(int i = 0 ; i < VMM_PAGE_COUNT ; i++) {

	}

	ptr_t i, j;
	// 0xC0000000 这个地址在页目录的索引，0x0300
	// ptr_t pgd_idx = VMM_PGD_INDEX(PAGE_OFFSET);
	ptr_t pgd_idx = VMM_PGD_INDEX(0);

	// 页目录
	for(i = pgd_idx, j = 0 ; i < VMM_KERNEL_PAGE_COUNT + pgd_idx ; i++, j++) {

		// 此处是内核虚拟地址，MMU 需要物理地址，所以减去偏移，下同
		pgd_kernel[i] = ( (ptr_t)pte_kernel[j] - PAGE_OFFSET) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;
	}
	printk_debug("22222222222\n");
	// 映射全部内存
	uint32_t * pte = (uint32_t *)pte_kernel;
	// 不映射第 0 页，便于跟踪 NULL 指针
	// 0x80000, 512MB
	for(i = 1 ; i < 0x38000000 ; i++) {
		pte[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;
	}
	printk_debug("333333333333\n");
	ptr_t pgd_kernel_phy_addr = (ptr_t)pgd_kernel - PAGE_OFFSET;

	// 注册页错误中断的处理函数 ( 14 是页故障的中断号 )
	register_interrupt_handler(INT_PAGE_FAULT, &page_fault);

	printk_debug("666666666666666\n");
	switch_pgd(pgd_kernel_phy_addr);
	printk_debug("777777777777777\n");
	printk_info("vmm_init\n");

	return;
}

void test() {

}

void map(pgd_t * pgd_now, ptr_t va, ptr_t pa, uint32_t flags) {
	uint32_t pgd_idx = VMM_PGD_INDEX(va);
	uint32_t pte_idx = VMM_PTE_INDEX(va);

	pte_t * pte = (pte_t *)(pgd_now[pgd_idx] & VMM_PAGE_MASK);
	if(!pte) {
		pte = (pte_t *)pmm_alloc(1);
		pgd_now[pgd_idx] = (ptr_t)pte | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;

		// 转换到内核线性地址并清 0
		pte = (pte_t *)( (ptr_t)pte + PAGE_OFFSET);
		bzero(pte, VMM_PAGE_SIZE);
	} else {
		// 转换到内核线性地址
		pte = (pte_t *)( (ptr_t)pte + PAGE_OFFSET);
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
	pte = (pte_t *)( (ptr_t)pte + PAGE_OFFSET);

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
	pte = (pte_t *)( (ptr_t)pte + PAGE_OFFSET);

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

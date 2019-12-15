
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.c for MRNIU/SimpleKernel.

#include "mem/vmm.h"
#include "debug.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

// 内核页目录区域, [128]
pgd_t pgd_kernel[PGD_SIZE] __attribute__((aligned(PAGE_SIZE)));

// 内核页表区域, [128][128], 16kb, 16384, 0x4000
static pte_t pte_kernel[PTE_COUNT][PTE_SIZE] __attribute__ ((aligned(PAGE_SIZE)));

void vmm_init(void) {
	// 0xC0000000 这个地址在页目录的索引，0x0300
	ptr_t kernel_pte_first_idx = PGD_INDEX(PAGE_OFFSET);

	// 映射所有内核地址，高位 1G
	ptr_t i, j;
	for (i = kernel_pte_first_idx, j = 0; i < PTE_COUNT + kernel_pte_first_idx; i++, j++) {
		// 此处是内核虚拟地址，MMU 需要物理地址，所以减去偏移，下同
		pgd_kernel[i] = ((ptr_t)pte_kernel[j] - PAGE_OFFSET) | PAGE_PRESENT | PAGE_WRITE;
	}

	uint32_t * pte = (uint32_t *)pte_kernel;
	// 不映射第 0 页，便于跟踪 NULL 指针
	for (i = 1; i < PTE_COUNT * PTE_SIZE; i++) {
		pte[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}

	ptr_t pgd_kernel_phy_addr = (ptr_t)pgd_kernel - PAGE_OFFSET;

	// 注册页错误中断的处理函数 ( 14 是页故障的中断号 )
	register_interrupt_handler(INT_PAGE_FAULT, &page_fault);

	switch_pgd(pgd_kernel_phy_addr);

	printk_info("vmm_init\n");

	return;
}

void switch_pgd(ptr_t pd) {
	__asm__ volatile ("mov %0, %%cr3" : : "r" (pd));
}

void page_fault(pt_regs_t * regs) {
	uint64_t cr2;
#ifdef __x86_64__
	asm volatile ( "movq %%cr2,%0" : "=r" ( cr2 ) );
#else
	asm volatile ( "mov %%cr2,%0" : "=r" ( cr2 ) );
#endif
	printk("Page fault at 0x%x, virtual faulting address 0x%x\n", regs->eip, cr2);
	printk_err("Error code: 0x%08X\n", regs->err_code);

	// bit 0 为 0 指页面不存在内存里
	if ( !( regs->err_code & 0x1 ) )
		printk_color(red, "Because the page wasn't present.\n");
	// bit 1 为 0 表示读错误，为 1 为写错误
	if (regs->err_code & 0x2)
		printk_err("Write error.\n");
	else
		printk_err("Read error.\n");
	// bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
	if (regs->err_code & 0x4)
		printk_err("In user mode.\n");
	else
		printk_err("In kernel mode.\n");
	// bit 3 为 1 表示错误是由保留位覆盖造成的
	if (regs->err_code & 0x8)
		printk_err("Reserved bits being overwritten.\n");
	// bit 4 为 1 表示错误发生在取指令的时候
	if (regs->err_code & 0x10)
		printk_err("The fault occurred during an instruction fetch.\n");
	while (1);
}

#ifdef __cplusplus
}
#endif

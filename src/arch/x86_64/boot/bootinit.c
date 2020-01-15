
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/bootinit.h"

// When writing a higher-half kernel, the steps required are:
// 1. Reserve some pages for the initial mappings, so it is possible to parse GRUB structures before memory management is enabled.
// 2. Create page tables that contain the page frames of the kernel image.
// 3. Create the boot page directory that contains the aforementioned page tables both in lower half and higher half.
// 4. Enable paging.
// 5. Jump to higher half.
// 6. Remove the lower half kernel mapping.

void enable_page(pgd_t * pgd) {
	// 设置临时页表
	__asm__ volatile ("mov %0, %%cr3" : : "r" (pgd) );
	uint32_t cr0;
	__asm__ volatile ("mov %%cr0, %0" : "=r" (cr0) );
	// 最高位 PG 位置 1，分页开启
	cr0 |= (1u << 31);
	__asm__ volatile ("mov %0, %%cr0" : : "r" (cr0) );
	return;
}

// 这时操作的是临时对象，正式初始化交给 kernel_main()
void mm_init() {
	// init 段, 4MB
	// 因为 mm_init 返回后仍然在 init 段，不映射的话会爆炸的
	pgd_tmp[0] = (ptr_t)pte_init | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	// 内核段 pgd_tmp[0x300], 4MB
	pgd_tmp[VMM_PGD_INDEX(KERNEL_BASE)] = (ptr_t)pte_kernel_tmp | VMM_PAGE_PRESENT | VMM_PAGE_RW;

	// 映射内核虚拟地址 4MB 到物理地址的前 4MB
	// 将每个页表项赋值
	// pgd_tmp[0] => pte_init
	for(uint32_t i = 0 ; i < VMM_PAGES_PRE_PAGE_TABLE ; i++) {
		pte_init[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	}

	// 映射 kernel 段 4MB
	// 映射虚拟地址 0xC0000000-0xC0400000 到物理地址 0x00000000-0x00400000
	// pgd_tmp[0x300] => pte_kernel
	for(uint32_t i = 0 ; i < VMM_PAGES_PRE_PAGE_TABLE ; i++) {
		pte_kernel_tmp[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_RW;
	}

	enable_page(pgd_tmp);

	return;
}

void switch_stack(ptr_t stack_top) {
	// 切换内核栈
	__asm__ volatile ("mov %0, %%esp" : : "r" (stack_top) );
	__asm__ volatile ("xor %%ebp, %%ebp" : :);
	return;
}

// 内核入口函数
void kernel_entry(ptr_t magic, ptr_t addr) {
	mm_init();
	switch_stack(kernel_stack_top);
	kernel_main(magic, KERNEL_BASE + addr);
	return;
}

#ifdef __cplusplus
}
#endif

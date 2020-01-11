
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
	pgd_tmp[0] = (ptr_t)pte_low | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;

	for(int i = 0 ; i < 4 ; ++i) {
		uint32_t pgd_idx = VMM_PGD_INDEX(PAGE_OFFSET + VMM_PTE_MAP_SIZE * i);
		pgd_tmp[pgd_idx] = ( (ptr_t)pte_high + VMM_PAGE_SIZE * i) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;
	}
	pgd_tmp[0] = ( (ptr_t)pte_high) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;

	// 映射内核虚拟地址 4MB 到物理地址的前 4MB
	// 因为 .init.text 段的代码在物理地址前 4MB 处(肯定不会超出这个范围)，
	// 开启分页后若此处不映射，代码执行立即会出错，离开 .init.text 段后的代码执行，
	// 不再需要映射物理前 4MB 的内存
	for(uint32_t i = 0 ; i < 1024 ; i++) {
		pte_low[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;
	}

	// 映射 0x00000000-0x01000000 的物理地址到虚拟地址 0xC0000000-0xC1000000
	for(uint32_t i = 0 ; i < 1024 * 4 ; i++) {
		pte_high[i] = (i << 12) | VMM_PAGE_PRESENT | VMM_PAGE_WRITE;
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
	kernel_main(magic, PAGE_OFFSET + addr);
	return;
}

#ifdef __cplusplus
}
#endif

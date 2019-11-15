
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// vmm.c for MRNIU/SimpleKernel.

#include "mm/include/vmm.h"
#include "include/debug.h"
#include "stdio.h"


#ifdef __cplusplus
extern "C" {
#endif

// When writing a higher-half kernel, the steps required are:
// 1. Reserve some pages for the initial mappings, so it is possible to parse GRUB structures before memory management is enabled.
// 2. Create page tables that contain the page frames of the kernel image.
// 3. Create the boot page directory that contains the aforementioned page tables both in lower half and higher half.
// 4. Enable paging.
// 5. Jump to higher half.
// 6. Remove the lower half kernel mapping.

// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0-640KB 肯定是空闲的
__attribute__((section(".init.data"))) pgd_t *pgd_tmp  = (pgd_t *)0x1000;
__attribute__((section(".init.data"))) pgd_t *pte_low  = (pgd_t *)0x2000;
__attribute__((section(".init.data"))) pgd_t *pte_high = (pgd_t *)0x3000;

void vmm_init(){
	pgd_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;

	for (int i = 0; i < 4; ++i) {
		uint32_t pgd_idx = PGD_INDEX(PAGE_OFFSET + PAGE_MAP_SIZE * i);
		pgd_tmp[pgd_idx] = ( (uint32_t)pte_high + PAGE_SIZE * i ) | PAGE_PRESENT | PAGE_WRITE;
	}

	// 映射内核虚拟地址 4MB 到物理地址的前 4MB
	// 因为 .init.text 段的代码在物理地址前 4MB 处(肯定不会超出这个范围)，
	// 开启分页后若此处不映射，代码执行立即会出错，离开 .init.text 段后的代码执行，
	// 不再需要映射物理前 4MB 的内存
	for (int i = 0; i < 1024; i++) {
		pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}

	// 映射 0x00000000-0x01000000 的物理地址到虚拟地址 0xC0000000-0xC1000000
	for (int i = 0; i < 1024 * 4; i++) {
		pte_high[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}

	// 设置临时页表
	__asm__ volatile ( "mov %0, %%cr3" : : "r" (pgd_tmp) );

	uint32_t cr0;

	__asm__ volatile ( "mov %%cr0, %0" : "=r" (cr0) );
	// 最高位 PG 位置 1，分页开启
	cr0 |= (1u << 31);
	__asm__ volatile ( "mov %0, %%cr0" : : "r" (cr0) );


	return;
}



#ifdef __cplusplus
}
#endif

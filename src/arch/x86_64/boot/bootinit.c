
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// bootinit.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/bootinit.h"

__attribute__((section(".init.text"))) inline void enable_page(uint32_t * pgd) {
	// 设置临时页表
	__asm__ volatile ( "mov %0, %%cr3" : : "r" (pgd) );
	uint32_t cr0;
	__asm__ volatile ( "mov %%cr0, %0" : "=r" (cr0) );
	// 最高位 PG 位置 1，分页开启
	cr0 |= (1u << 31);
	__asm__ volatile ( "mov %0, %%cr0" : : "r" (cr0) );
	return;
}

// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0-640KB 肯定是空闲的
__attribute__( ( section(".init.data") ) ) pgd_t *pgd_tmp  = (pgd_t *)0x1000;
__attribute__( ( section(".init.data") ) ) pte_t *pte_low  = (pte_t *)0x2000;
__attribute__( ( section(".init.data") ) ) pte_t *pte_high = (pte_t *)0x3000;

// 这时操作的是临时对象，正式初始化交给 kernel_main()
__attribute__((section(".init.text"))) void mm_init() {
	pgd_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;

	for (int i = 0; i < 4; ++i) {
		uint32_t pgd_idx = PGD_INDEX(PAGE_MAP_SIZE * i);
		pgd_tmp[pgd_idx] = ( (uint32_t)pte_high + PAGE_SIZE * i ) | PAGE_PRESENT | PAGE_WRITE;
	}
	pgd_tmp[0] = ( (uint32_t)pte_high ) | PAGE_PRESENT | PAGE_WRITE;

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
	enable_page(pgd_tmp);
	printk_color(COL_INFO, "[INFO] ");
	printk("mm_init\n");
	return;
}

// 内核入口函数
__attribute__((section(".init.text"))) void kernel_entry(uint32_t magic, uint32_t addr) {
	console_init(); // 控制台初始化
	multiboot2_init(magic, addr); // 从 multiboot 获得系统初始信息
	gdt_init(); // GDT 初始化
	idt_init(); // IDT 初始化
	clock_init(); // 时钟初始化
	keyboard_init(); // 键盘初始化
	mm_init();
	debug_init(magic, addr);
	showinfo();

	// test();

	kernel_main(magic, addr);
	return;
}


#ifdef __cplusplus
}
#endif

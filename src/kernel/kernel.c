
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"
#include "../test/test.h"

#define DEBUG 1

// NOTE!
// 流程
// 链接器确定入口
// 入口汇编负责 multiboot2 相关设置与进入 PG（PE 由 grub 负责）
//		建立临时页表，开启分页，进入内核后临时页表被弃置
// 进入内核
// 内存管理初始化
// etc.


// 开启分页机制之后的内核栈
// char kern_stack[STACK_SIZE];

// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0-640KB 肯定是空闲的
__attribute__((section(".init.data"))) uint32_t *pgd_tmp  = (uint32_t *)0x1000;
__attribute__((section(".init.data"))) uint32_t *pte_low  = (uint32_t *)0x2000;
__attribute__((section(".init.data"))) uint32_t *pte_high = (uint32_t *)0x3000;

// 内核入口函数
__attribute__((section(".init.text"))) void kern_entry(uint32_t magic, uint32_t addr)
{
		pgd_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;
		pgd_tmp[PGD_INDEX(PAGE_OFFSET)] = (uint32_t)pte_high | PAGE_PRESENT | PAGE_WRITE;

// 映射内核虚拟地址 4MB 到物理地址的前 4MB
		int i;
		for (i = 0; i < 1024; i++) {
				pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
		}

// 映射 0x00000000-0x00400000 的物理地址到虚拟地址 0xC0000000-0xC0400000
		for (i = 0; i < 1024; i++) {
				pte_high[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
		}

// 设置临时页表
		asm volatile ("mov %0, %%cr3" : : "r" (pgd_tmp));

		uint32_t cr0;

		// 启用分页，将 cr0 寄存器的分页位置为 1 就好
		asm volatile ("mov %%cr0, %0" : "=r" (cr0));
		cr0 |= 0x80000000;
		asm volatile ("mov %0, %%cr0" : : "r" (cr0));

		// 切换内核栈
		// uint32_t kern_stack_top = ((uint32_t)kern_stack + STACK_SIZE) & 0xFFFFFFF0;
		// asm volatile ("mov %0, %%esp\n\t"
		// "xor %%ebp, %%ebp" : : "r" (kern_stack_top));

		// bug，必须访问这两个变量才能正确传递
		// 原因是上面的栈切换
		printk_color(red, "magic: %x", magic);
		printk_color(red, "addr: %x", addr);

		// 调用内核初始化函数
		// kernel_main(magic, addr);
}

// 内核入口
void kernel_main(uint32_t magic, uint32_t addr) {
		console_init();       // 控制台初始化
		gdt_init();        // GDT 初始化
		idt_init();       // IDT 初始化
		clock_init();       // 时钟初始化
		keyboard_init();       // 键盘初始化
		multiboot2_init(magic, addr); // 包括 pmm_init 与 vmm_init
		// debug_init(magic, addr);
		showinfo();
		test();
		// uint64_t tmp=pmm_alloc_page();


		while(1);

		return;
}

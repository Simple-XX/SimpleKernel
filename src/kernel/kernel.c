
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"
#include "../test/test.h"

void panic(const char *msg){
		printk("*** System panic: %s\n", msg);
		print_stack_trace();
		printk("***\n");

		// 致命错误发生后打印栈信息后停止在这里
		while(1)
				cpu_hlt();
}

const char * elf_lookup_symbol(uint32_t addr, elf_t *elf){
		printk("%x\n", elf->symtabsz);
		for (uint32_t i = 0; i < (elf->symtabsz / sizeof(Elf32_Sym)); i++) {
				printk("66666666666\n");
				if (ELF32_ST_TYPE(elf->symtab[i].st_info) != 0x2) {
						continue;
				}
				// 通过函数调用地址查到函数的名字(地址在该函数的代码段地址区间之内)
				if ( (addr >= elf->symtab[i].st_value) && (addr < (elf->symtab[i].st_value + elf->symtab[i].st_size)) ) {
						return (const char *)((uint32_t)elf->strtab + elf->symtab[i].st_name);
				}
		}
		return NULL;
}

void print_stack_trace(void){
		uint32_t *ebp, *eip;
		__asm__ volatile ("mov %%ebp, %0" : "=r" (ebp));
		while (*ebp) {
				eip = ebp + 1;
				printk("   [0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf));
				ebp = (uint32_t*)*ebp;
		}
}



// 内核入口
void kernel_main(uint32_t magic, uint32_t addr) {
		console_init();       // 控制台初始化

		gdt_init();        // GDT 初始化
		idt_init();       // IDT 初始化
		clock_init();       // 时钟初始化
		keyboard_init();       // 键盘初始化
		pmm_init((multiboot_tag_t *)addr);
		debug_init(magic, addr);
		showinfo();
		// test();
		panic("test");

		while(1);

		return;
}

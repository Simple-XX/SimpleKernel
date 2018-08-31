
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"


void test(pt_regs_t * regs){
	unsigned char x;
	x=inb(0x60);
	printk("%c\n", x);
	outb(0x20, 0x20);
	UNUSED(regs);
}

void keyboard_init(void){
	printk("initialize keyboard\n");
	//register_interrupt_handler(IRQ1, &keyboard_callback);
	register_interrupt_handler(IRQ1, &test);
}
// 内核入口
void kernel_main()
{
	// 系统初始化
	debug_init();
	terminal_init();	// 控制台初始化
	gdt_init();	// GDT 初始化
	idt_init();	// IDT 初始化
	clock_init();	// 时钟初始化
	keyboard_init(); // 键盘初始化
	showinfo();
	//asm("int $33");

	while(1){
		
	}




	printk_color(white, "\nEnd.\n");
	return;
}

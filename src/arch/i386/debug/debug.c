
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-5.html
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"

void debug_init(){

  return;
}

void print_registers(pt_regs_t * regs){
	printk("(registers\n");
	printk("\tedi %x\n\tesi %x\n\tebp %x\n\tesp %x\n\tebx %x\n\tedx %x\n\tecx %x\n\teax %x\n",
		regs->edi, regs->esi, regs->ebp, regs->esp, regs->ebx, regs->edx, regs->ecx, regs->eax);
	printk("\tds %x\n\tint_no %x\n\terr_code %x\n\teip %x\n\tcs %x\n\teflags %x\n\tuseresp %x\n\tss %x",
		      regs->ds, regs->int_no, regs->err_code,regs->eip, regs->cs, regs->eflags,
          regs->useresp, regs->ss);
	printk(")\n");
}

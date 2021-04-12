
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot.s for Simple-XX/SimpleKernel.

.section .init
 
.option norvc
 
.type _start, @function
.global _start
_start:
	.cfi_startproc
 
.option push
.option norelax
	la gp, global_pointer
.option pop
 
	/* Reset satp */
	csrw satp, zero
 
	/* Setup stack */
	la sp, stack_top
 
	/* Clear the BSS section */
	la t5, kernel_bss_start
	la t6, kernel_bss_end
bss_clear:
	sd zero, (t5)
	addi t5, t5, 8
	bgeu t5, t6, bss_clear
 
	la t0, kernel_main
	csrw mepc, t0

	/* Jump to kernel! */
	tail kernel_main

	.cfi_endproc

.end
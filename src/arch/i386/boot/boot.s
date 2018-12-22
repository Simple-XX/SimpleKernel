# Copy from osdev.org Bare_Bones:
# (https://wiki.osdev.org/Bare_Bones).

# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
# boot.s for MRNIU/SimpleKernel.

/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM


.bss
.align 16
stack_bottom:
.skip 16384 # 16 KB 0x4000  STACK_SIZE
stack_top:

.text
.global _start
.type _start, @function

_start:

	# 初始化栈指针
	mov $stack_top, %esp

	# 还原 EFLAGES
	pushl $0
	popf

	# Push the pointer to the Multiboot information structure.
	push %ebx

	# Push the magic value.
	push %eax
	# 跳转到 C 入口函数
	call kernel_main
	cli

1:hlt
	jmp 1b

.size _start, . - _start

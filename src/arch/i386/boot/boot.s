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

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KB
stack_top:
glb_mboot_ptr:

.section .text
.global glb_mboot_ptr
.global _start
.type _start, @function

_start:
	mov $stack_top, %esp
	mov glb_mboot_ptr, %ebx  /* 将 ebx 中存储的指针存入全局变量 */
	call kernel_main
	cli
1:hlt
	jmp 1b

.size _start, . - _start


# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
# boot.s for MRNIU/SimpleKernel.


.set STACK_SIZE, 0x4000

.text
.align 8
.global start, _start
.global multiboot2_info
.global multiboot2_magic
.extern kernel_main

.align 8
multiboot_header:
  .long 0xe85250d6
  .long 0
  .long multiboot_header_end - multiboot_header
  .long -(0xe85250d6 + 0 + multiboot_header_end - multiboot_header)

	.align 8
	.short 0
  .short 0
  .long 8

.align 8
start:

.align 8
multiboot_header_end:

.align 8
multiboot_entry:
  mov $(stack + STACK_SIZE), %esp
  push $0
  popf
  push %ebx
	push %eax
  call kernel_main
.comm   stack, STACK_SIZE

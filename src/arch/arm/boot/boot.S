
// This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/Raspberry_Pi_Bare_Bones
// boot.S for Simple-XX/SimpleKernel.

.code 32

.section .text.init
 
// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42 - machine id
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
.globl _start
// 规定起始地址
.org 0x8000
_start:
	// 关闭除 cpu0 以外的其它 core
	mrc p15, 0, r5, c0, c0, 5
	and r5, r5, #3
	cmp r5, #0
	bne halt
 
	// 设置栈
	ldr r5, =_start
	mov sp, r5
 
	// 清空 bss
	ldr r4, = kernel_bss_start
	ldr r9, = kernel_bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f
 
1:
	// store multiple at r4.
	stmia r4!, {r5-r8}
 
	// If we are still below bss_end, loop.
2:
	cmp r4, r9
	blo 1b
	// 进入 kernel_main
	ldr r3, = kernel_main
	blx r3
 
// halt
halt:
	wfe
	b halt


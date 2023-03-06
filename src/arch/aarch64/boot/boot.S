
// This file is a part of Simple-XX/SimpleKernel 
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://wiki.osdev.org/Raspberry_Pi_Bare_Bones
// boot.S for Simple-XX/SimpleKernel.

.section .init
 
// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42 - machine id
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
.globl _start
.type _start, @function
.extern kernel_main
// 规定起始地址
.org 0x8000
_start:
    // 设置栈
    ldr x5, =_start
    mov sp, x5
    // 进入 kernel_main
    bl kernel_main
 
// halt
halt:
    wfe
    b halt

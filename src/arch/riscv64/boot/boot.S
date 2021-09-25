
// This file is a part of Simple-XX/SimpleKernel 
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot.S for Simple-XX/SimpleKernel.
// 启动代码，进行一些设置后跳转到 kernel_main

.section .init
.globl _start
.type _start, @function
.extern kernel_main
_start:
    // 设置栈地址
    la sp, stack_top
    // 跳转到 C 代码执行
    call kernel_main

// 声明所属段
.section .bss
// 16 字节对齐
.align 16
.global stack_top
stack_top:
    // 跳过 16KB
    .space 4096 * 4

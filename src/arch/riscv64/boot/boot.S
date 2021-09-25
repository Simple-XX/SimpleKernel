
// This file is a part of Simple-XX/SimpleKernel 
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot.S for Simple-XX/SimpleKernel.
// 启动代码，进行一些设置后跳转到 kernel_main

.section .init
.globl _start
.type _start, @function
.extern kernel_main
.extern cpp_init
_start:
    // 设置栈地址
    la    sp, stack_top
    # 在调用 cpp_init 之前保存由 opensbi 传递的 hart 和 dtb 地址参数
    addi sp, sp, -16
    sd a1, (0)(sp)
    sd a0, (8)(sp)
    # 初始化 C++
    call cpp_init
    # cpp_init 结束后恢复
    ld a0, (8)(sp)
    ld a1, (0)(sp)
    addi sp, sp, 16
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


# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot.S for Simple-XX/SimpleKernel.

.section .init
.globl _start
.type _start, @function
.extern kernel_main
.extern cpp_init
_start:
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
    call kernel_main

.section .bss
.align 16
.global stack_top
stack_top:
    .space 4096 * 4

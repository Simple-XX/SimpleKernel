
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot.S for Simple-XX/SimpleKernel.

.section .init
.globl _start
.type _start, @function
.extern kernel_main
.extern cpp_init
.extern dtb_addr
.extern dtb_init_hart
_start:
    # 保存 sbi 传递的参数
    # 将 a0 的值传递给 dtb_init_hart
    sw a0, dtb_init_hart, t0
    # 将 a1 的值传递给 dtb_addr
    sw a1, dtb_addr, t0
    # 设置栈
    la sp, stack_top
    # 初始化 C++
    call cpp_init
    call kernel_main

.section .bss
.align 16
.global stack_top
stack_top:
    .space 4096 * 4

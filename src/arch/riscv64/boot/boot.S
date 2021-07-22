
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot.S for Simple-XX/SimpleKernel.

.section .init
.globl _start
.type _start, @function
.extern kernel_main
_start:
    la sp, stack_top
    call kernel_main

.section .bss
.align 16
.global stack_top
stack_top:
    .space 4096 * 4

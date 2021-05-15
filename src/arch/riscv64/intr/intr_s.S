
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# intr_s.s for Simple-XX/SimpleKernel.

.equ REGBYTES, 8
.equ SAVE_REGS, 32
.equ CONTEXT_SIZE, (SAVE_REGS * REGBYTES)

.macro lx a, b
ld \a, \b
.endm

.macro sx a, b
sd \a, \b
.endm

.macro lxsp a, b
ld \a, ((\b)*REGBYTES)(sp)
.endm

.macro sxsp a, b
sd \a, ((\b)*REGBYTES)(sp)
.endm

.section .text

# push all registers, call trap_handler(), restore, return.
.extern irq_handler
.globl trap_entry
.extern trap_handler
.align 4
trap_entry:
    # make room to save registers.
    addi sp, sp, -CONTEXT_SIZE

    # save the registers.
    sxsp x0, 0,
    sxsp x1, 1,
    sxsp x2, 2,
    sxsp x3, 3,
    sxsp x4, 4,
    sxsp x5, 5,
    sxsp x6, 6,
    sxsp x7, 7,
    sxsp x8, 8,
    sxsp x9, 9,
    sxsp x10, 10
    sxsp x11, 11
    sxsp x12, 12
    sxsp x13, 13
    sxsp x14, 14
    sxsp x15, 15
    sxsp x16, 16
    sxsp x17, 17
    sxsp x18, 18
    sxsp x19, 19
    sxsp x20, 20
    sxsp x21, 21
    sxsp x22, 22
    sxsp x23, 23
    sxsp x24, 24
    sxsp x25, 25
    sxsp x26, 26
    sxsp x27, 27
    sxsp x28, 28
    sxsp x29, 29
    sxsp x30, 30
    sxsp x31, 31

    # in intr.cpp
    # 当异常发生时，跳转到 trap_handler 
    call trap_handler

    # restore registers.
    lxsp x0, 0
    lxsp x1, 1
    lxsp x2, 2
    lxsp x3, 3
    lxsp x4, 4
    lxsp x5, 5
    lxsp x6, 6
    lxsp x7, 7
    lxsp x8, 8
    lxsp x9, 9
    lxsp x10, 10
    lxsp x11, 11
    lxsp x12, 12
    lxsp x13, 13
    lxsp x14, 14
    lxsp x15, 15
    lxsp x16, 16
    lxsp x17, 17
    lxsp x18, 18
    lxsp x19, 19
    lxsp x20, 20
    lxsp x21, 21
    lxsp x22, 22
    lxsp x23, 23
    lxsp x24, 24
    lxsp x25, 25
    lxsp x26, 26
    lxsp x27, 27
    lxsp x28, 28
    lxsp x29, 29
    lxsp x30, 30
    lxsp x31, 31

    addi sp, sp, CONTEXT_SIZE

    # return to whatever we were doing in the kernel.
    mret

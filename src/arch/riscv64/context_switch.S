
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// context_switch.S for Simple-XX/SimpleKernel.

// 寄存器长度，8 字节
.equ REG_BYTES, 8
// 保存所有寄存器
.equ CALLEE_SAVE_REGS, 25
.equ SAVE_REGS, 32
// 需要的大小
.equ CONTEXT_SIZE, (SAVE_REGS * REG_BYTES)
.equ CALLEE_SIZE, (CALLEE_SAVE_REGS * REG_BYTES)

// 一些宏
.macro sd_base a, b, c
sd \a, ((\b)*REG_BYTES)(\c)
.endm

.macro ld_base a, b, c
ld \a, ((\b)*REG_BYTES)(\c)
.endm

.macro fsd_base a, b, c
fsd \a, ((\b)*REG_BYTES)(\c)
.endm

.macro fld_base a, b, c
fld \a, ((\b)*REG_BYTES)(\c)
.endm

// 保存
.macro callee_regs_save base
    sd_base sp, 0, \base
    sd_base s0, 1, \base
    sd_base s1, 2, \base
    sd_base s2, 3, \base
    sd_base s3, 4, \base
    sd_base s4, 5, \base
    sd_base s5, 6, \base
    sd_base s6, 7, \base
    sd_base s7, 8, \base
    sd_base s8, 9, \base
    sd_base s9, 10, \base
    sd_base s10, 11, \base
    sd_base s11, 12, \base
    fsd_base fs0, 13, \base
    fsd_base fs1, 14, \base
    fsd_base fs2, 15, \base
    fsd_base fs3, 16, \base
    fsd_base fs4, 17, \base
    fsd_base fs5, 18, \base
    fsd_base fs6, 19, \base
    fsd_base fs7, 20, \base
    fsd_base fs8, 21, \base
    fsd_base fs9, 22, \base
    fsd_base fs10, 23, \base
    fsd_base fs11, 24, \base
.endm

// 恢复
.macro callee_regs_load base
    ld_base sp, 0, \base
    ld_base s0, 1, \base
    ld_base s1, 2, \base
    ld_base s2, 3, \base
    ld_base s3, 4, \base
    ld_base s4, 5, \base
    ld_base s5, 6, \base
    ld_base s6, 7, \base
    ld_base s7, 8, \base
    ld_base s8, 9, \base
    ld_base s9, 10, \base
    ld_base s10, 11, \base
    ld_base s11, 12, \base
    fld_base fs0, 13, \base
    fld_base fs1, 14, \base
    fld_base fs2, 15, \base
    fld_base fs3, 16, \base
    fld_base fs4, 17, \base
    fld_base fs5, 18, \base
    fld_base fs6, 19, \base
    fld_base fs7, 20, \base
    fld_base fs8, 21, \base
    fld_base fs9, 22, \base
    fld_base fs10, 23, \base
    fld_base fs11, 24, \base
.endm


// 保存 context_t
.macro context_save base
    sd_base ra, 0, \base
    addi \base, \base, 8
    callee_regs_save \base
    addi \base, \base, CALLEE_SIZE
    csrr t6, satp
    sd_base t6, 0, \base
    addi \base, \base, 8
    csrr t6, sepc
    sd_base t6, 0, \base
    addi \base, \base, 8
    csrr t6, sscratch
    sd_base t6, 0, \base
.endm

// 恢复 context_t
.macro context_load base
    ld_base ra, 0, \base
    addi \base, \base, 8
    callee_regs_load \base
    addi \base, \base, CALLEE_SIZE
    ld_base t6, 0, \base
    csrw satp, t6
    addi \base, \base, 8
    ld_base t6, 0, \base
    csrw sepc, t6
    addi \base, \base, 8
    ld_base t6, 0, \base
    csrw sscratch, t6
.endm

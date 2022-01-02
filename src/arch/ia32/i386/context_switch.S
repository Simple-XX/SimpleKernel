
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

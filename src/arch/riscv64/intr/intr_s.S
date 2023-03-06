
/**
 * @file intr_s.S
 * @brief 中断处理入口
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-01-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "context.S"

.section .text
// 保存所有寄存器
.globl trap_entry
.extern trap_handler
.align 4
trap_entry:
    // 将所有寄存器保存到栈上
    // 在栈上留出保存寄存器的空间
    addi sp, sp, -ALL_SIZE
    all_regs_save sp

    // 调用 intr.cpp: trap_handler
    // 传递参数
    csrr a0, sepc
    csrr a1, stval
    csrr a2, scause
    mv   a3, sp
    csrr a4, sie
    csrr a5, sstatus
    csrr a6, satp
    csrr a7, sscratch
    jal trap_handler

    // 从栈上恢复所有寄存器
    all_regs_load sp
    // 释放栈上用于保存寄存器的空间
    addi sp, sp, ALL_SIZE

    // 跳转到 sepc 处执行
    sret

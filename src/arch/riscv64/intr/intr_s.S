
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

#include "context_switch.S"

.section .text
// 保存所有寄存器
.extern irq_handler
.globl trap_entry
.extern trap_handler
.align 4
trap_entry:
    // 保存到栈上的特殊处理，预留空间
    addi sp, sp, -CALLER_SIZE
    context_save sp

    // 调用 intr.cpp: trap_handler
    // 传递参数
    csrr a0, sepc
    csrr a1, stval
    csrr a2, scause
    mv   a3, sp
    csrr a4, sstatus
    csrr a5, sscratch
    jal trap_handler

    // 保存到栈上的特殊处理，回收空间
    context_load sp
    addi sp, sp, CALLER_SIZE

    // 跳转到 sepc 处执行
    sret

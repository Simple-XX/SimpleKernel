
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

// clang-format off

.macro SAVE_ALL
    # 通过原子操作交换 sp, sscratch
    # 实际上是将右侧寄存器的值写入中间 csr
    # 并将中间 csr 的值写入左侧寄存器
    csrrw sp, sscratch, sp

    # 两种情况接下来都是在内核栈上保存上下文环境
    bnez sp, trap_from_user
trap_from_kernel:
    csrr sp, sscratch
trap_from_user:
    # 提前分配栈帧
    addi sp, sp, -36*kRegsBytes
    # 按照地址递增的顺序，保存除x0, x2之外的通用寄存器
    # x0 恒为 0 不必保存
    # x2 为 sp 寄存器，需特殊处理
    SdBase x1, 1, sp
    SdBase x3, 3, sp
    SdBase x4, 4, sp
    SdBase x5, 5, sp
    SdBase x6, 6, sp
    SdBase x7, 7, sp
    SdBase x8, 8, sp
    SdBase x9, 9, sp
    SdBase x10, 10, sp
    SdBase x11, 11, sp
    SdBase x12, 12, sp
    SdBase x13, 13, sp
    SdBase x14, 14, sp
    SdBase x15, 15, sp
    SdBase x16, 16, sp
    SdBase x17, 17, sp
    SdBase x18, 18, sp
    SdBase x19, 19, sp
    SdBase x20, 20, sp
    SdBase x21, 21, sp
    SdBase x22, 22, sp
    SdBase x23, 23, sp
    SdBase x24, 24, sp
    SdBase x25, 25, sp
    SdBase x26, 26, sp
    SdBase x27, 27, sp
    SdBase x28, 28, sp
    SdBase x29, 29, sp
    SdBase x30, 30, sp
    SdBase x31, 31, sp

    csrrw s0, sscratch, x0
    # 分别将四个寄存器的值保存在 s1,s2,s3,s4 中
    csrr s1, sstatus
    csrr s2, sepc
    csrr s3, stval
    csrr s4, scause

    # 将 s0 保存在栈上
    SdBase s0, 2, sp
    # 将 s1,s2,s3,s4 保存在栈上
    SdBase s1, 32, sp
    SdBase s2, 33, sp
    SdBase s3, 34, sp
    SdBase s4, 35, sp
.endm

.macro RESTORE_ALL
    # s1 = sstatus
    LdBase s1, 32, sp
    # s2 = sepc
    LdBase s2, 33, sp
    # 恢复 sstatus, sepc 寄存器
    csrw sstatus, s1
    csrw sepc, s2

    # 恢复除 x0, x2(sp) 之外的通用寄存器
    LdBase x1, 1, sp
    LdBase x3, 3, sp
    LdBase x4, 4, sp
    LdBase x5, 5, sp
    LdBase x6, 6, sp
    LdBase x7, 7, sp
    LdBase x8, 8, sp
    LdBase x9, 9, sp
    LdBase x10, 10, sp
    LdBase x11, 11, sp
    LdBase x12, 12, sp
    LdBase x13, 13, sp
    LdBase x14, 14, sp
    LdBase x15, 15, sp
    LdBase x16, 16, sp
    LdBase x17, 17, sp
    LdBase x18, 18, sp
    LdBase x19, 19, sp
    LdBase x20, 20, sp
    LdBase x21, 21, sp
    LdBase x22, 22, sp
    LdBase x23, 23, sp
    LdBase x24, 24, sp
    LdBase x25, 25, sp
    LdBase x26, 26, sp
    LdBase x27, 27, sp
    LdBase x28, 28, sp
    LdBase x29, 29, sp
    LdBase x30, 30, sp
    LdBase x31, 31, sp

    LdBase x2, 2, sp
.endm

.section .text
.align 4
.extern TrapHandler
// 保存所有寄存器
.global TrapEntry
TrapEntry:
    // 将所有寄存器保存到栈上
    //SaveAllRegs
    SAVE_ALL

    // 调用 intr.cpp: TrapHandler
    // 传递参数
    csrr a0, sepc
    csrr a1, stval
    csrr a2, scause
    mv   a3, sp
    csrr a4, sie
    csrr a5, sstatus
    csrr a6, satp
    csrr a7, sscratch
    jal TrapHandler

    // 从栈上恢复所有寄存器
    //LoadAllRegs
    RESTORE_ALL

    csrr a0, sepc
    addi a0, a0, 2
    csrw sepc, a0

    // 跳转到 sepc 处执行
    sret

/*

.section .text
.globl __alltraps
.align 4
__alltraps:
    SAVE_ALL
    mv a0, sp
    jal TrapHandler

    RESTORE_ALL

    csrr a0, sepc
    addi a0, a0, 2
    csrw sepc, a0

    sret
*/
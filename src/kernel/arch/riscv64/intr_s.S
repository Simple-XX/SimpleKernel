
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

.section .text
.align 4
.extern TrapHandler
// 保存所有寄存器
.global TrapEntry
TrapEntry:
    // 将所有寄存器保存到栈上
    SaveAllRegs

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
    LoadAllRegs

    csrr a0, sepc
    addi a0, a0, 2
    csrw sepc, a0

    // 跳转到 sepc 处执行
    sret


# 常量：表示每个寄存器占的字节数，由于是64位，都是8字节
.equ XLENB, 8

# 将地址 sp+8*a2 处的值 load 到寄存器 a1 内
.macro LOAD a1, a2
    ld \a1, \a2*XLENB(sp)
.endm

# 将寄存器 a1 内的值 store 到地址 sp+8*a2 内
.macro STORE a1, a2
    sd \a1, \a2*XLENB(sp)
.endm

.macro SAVE_ALL
    # 通过原子操作交换 sp, sscratch
    # 实际上是将右侧寄存器的值写入中间 csr
    # 并将中间 csr 的值写入左侧寄存器
    csrrw sp, sscratch, sp

    # 如果 sp=0 ，说明交换前 sscratch=0
    #   则说明从内核态进入中断，不用切换栈
    #   因此不跳转，继续执行 csrr 再将 sscratch 的值读回 sp
    #   此时 sp,sscratch 均保存内核栈

    #   否则 说明sp!=0，说明从用户态进入中断，要切换栈
    #   由于 sscratch 规定，二者交换后
    #   此时 sp 为内核栈， sscratch 为用户栈
    #   略过 csrr 指令

    # 两种情况接下来都是在内核栈上保存上下文环境
    bnez sp, trap_from_user
trap_from_kernel:
    csrr sp, sscratch
trap_from_user:
    # 提前分配栈帧
    addi sp, sp, -36*XLENB
    # 按照地址递增的顺序，保存除x0, x2之外的通用寄存器
    # x0 恒为 0 不必保存
    # x2 为 sp 寄存器，需特殊处理
    STORE x1, 1
    STORE x3, 3
    STORE x4, 4
    STORE x5, 5
    STORE x6, 6
    STORE x7, 7
    STORE x8, 8
    STORE x9, 9
    STORE x10, 10
    STORE x11, 11
    STORE x12, 12
    STORE x13, 13
    STORE x14, 14
    STORE x15, 15
    STORE x16, 16
    STORE x17, 17
    STORE x18, 18
    STORE x19, 19
    STORE x20, 20
    STORE x21, 21
    STORE x22, 22
    STORE x23, 23
    STORE x24, 24
    STORE x25, 25
    STORE x26, 26
    STORE x27, 27
    STORE x28, 28
    STORE x29, 29
    STORE x30, 30
    STORE x31, 31

    # 若从内核态进入中断，此时 sscratch 为内核栈地址
    # 若从用户态进入中断，此时 sscratch 为用户栈地址
    # 将 sscratch 的值保存在 s0 中，并将 sscratch 清零
    csrrw s0, sscratch, x0
    # 分别将四个寄存器的值保存在 s1,s2,s3,s4 中
    csrr s1, sstatus
    csrr s2, sepc
    csrr s3, stval
    csrr s4, scause

    # 将 s0 保存在栈上
    STORE s0, 2
    # 将 s1,s2,s3,s4 保存在栈上
    STORE s1, 32
    STORE s2, 33
    STORE s3, 34
    STORE s4, 35
.endm



.macro RESTORE_ALL
    # s1 = sstatus
    LOAD s1, 32
    # s2 = sepc
    LOAD s2, 33
    # 我们可以通过另一种方式判断是从内核态还是用户态进入中断
    # 如果从内核态进入中断， sstatus 的 SPP 位被硬件设为 1
    # 如果从用户态进入中断， sstatus 的 SPP 位被硬件设为 0
    # 取出 sstatus 的 SPP
    andi s0, s1, 1 << 8
    # 若 SPP=0 ， 从用户态进入中断，进行 _to_user 额外处理
    bnez s0, _to_kernel
_to_user:
    # 释放在内核栈上分配的内存
    addi s0, sp, 36 * XLENB
    # RESTORE_ALL 之后，如果从用户态进入中断
    # sscratch 指向用户栈地址！
    # 现在令 sscratch 指向内核栈顶地址
    # 如果是从内核态进入中断，在 SAVE_ALL 里面
    # 就把 sscratch 清零了，因此保证了我们的规定
    csrw sscratch, s0
_to_kernel:
    # 恢复 sstatus, sepc 寄存器
    csrw sstatus, s1
    csrw sepc, s2

    # 恢复除 x0, x2(sp) 之外的通用寄存器
    LOAD x1, 1
    LOAD x3, 3
    LOAD x4, 4
    LOAD x5, 5
    LOAD x6, 6
    LOAD x7, 7
    LOAD x8, 8
    LOAD x9, 9
    LOAD x10, 10
    LOAD x11, 11
    LOAD x12, 12
    LOAD x13, 13
    LOAD x14, 14
    LOAD x15, 15
    LOAD x16, 16
    LOAD x17, 17
    LOAD x18, 18
    LOAD x19, 19
    LOAD x20, 20
    LOAD x21, 21
    LOAD x22, 22
    LOAD x23, 23
    LOAD x24, 24
    LOAD x25, 25
    LOAD x26, 26
    LOAD x27, 27
    LOAD x28, 28
    LOAD x29, 29
    LOAD x30, 30
    LOAD x31, 31

    # 如果从用户态进入中断， sp+2*8 地址处保存用户栈顶地址
    # 切换回用户栈
    # 如果从内核态进入中断， sp+2*8 地址处保存内核栈顶地址
    # 切换回内核栈
    LOAD x2, 2
.endm


.section .text
.globl __alltraps
.align 4
__alltraps:
    SAVE_ALL
    mv a0, sp
    jal TrapHandler

    .globl __trapret
__trapret:
    RESTORE_ALL

    csrr a0, sepc
    addi a0, a0, 2
    csrw sepc, a0

    sret

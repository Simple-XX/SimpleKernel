
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

.equ REGBYTES, 8
.equ SAVE_REGS, 16
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

// push all registers, call trap_handler(), restore, return.
.extern irq_handler
.globl trap_entry
.extern trap_handler
.align 4
trap_entry:
    // make room to save registers.
    addi sp, sp, -CONTEXT_SIZE

    sxsp ra, 0
    sxsp a0, 1
    sxsp a1, 2
    sxsp a2, 3
    sxsp a3, 4
    sxsp a4, 5
    sxsp a5, 6
    sxsp a6, 7
    sxsp a7, 8
    sxsp t0, 9
    sxsp t1, 10
    sxsp t2, 11
    sxsp t3, 12
    sxsp t4, 13
    sxsp t5, 14
    sxsp t6, 15

    // Invoke the handler.
    // intr.cpp
    csrr a0, scause
    csrr a1, sepc
    csrr a2, stval
    jal trap_handler

    // Restore registers.
    lxsp ra, 0
    lxsp a0, 1
    lxsp a1, 2
    lxsp a2, 3
    lxsp a3, 4
    lxsp a4, 5
    lxsp a5, 6
    lxsp a6, 7
    lxsp a7, 8
    lxsp t0, 9
    lxsp t1, 10
    lxsp t2, 11
    lxsp t3, 12
    lxsp t4, 13
    lxsp t5, 14
    lxsp t6, 15

    addi sp, sp, CONTEXT_SIZE

    // return to whatever we were doing in the kernel.
    sret

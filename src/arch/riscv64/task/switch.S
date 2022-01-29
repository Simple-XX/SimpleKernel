
/**
 * @file switch.S
 * @brief 上下文切换
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-02<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "context.S"

/**
 * @brief 初始化第一个进程的上下文
 * @param  _context         第一个进程
 */
// void context_init(context_t *_context);
.globl context_init
.align 4
context_init:
    // 跳过保存 coreid 的 8bytes
    addi a0, a0, REG_BYTES
    sd_base ra, 0, a0

    addi a0, a0, REG_BYTES
    callee_regs_save a0

    addi a0, a0, CALLEE_SIZE
    csrr t6, satp
    sd_base t6, 0, a0

    addi a0, a0, REG_BYTES
    csrr t6, sepc
    sd_base t6, 0, a0

    addi a0, a0, REG_BYTES
    csrr t6, sstatus
    sd_base t6, 0, a0

    addi a0, a0, REG_BYTES
    csrr t6, sie
    sd_base t6, 0, a0

    addi a0, a0, REG_BYTES
    csrr t6, sip
    sd_base t6, 0, a0

    addi a0, a0, REG_BYTES
    ld_base t6, 0, a0
    csrw sscratch, t6

    ret

/**
 * @brief 上下文切换
 * @param  _old             当前进程
 * @param  _new             下一个进程
 */
// void switch_context(context_t *_old, context_t *_new);
.globl switch_context
.align 4
switch_context:
    // 保存 _old
    context_save a0
    // 恢复 _new
    context_load a1
    // 跳转到 _new->ra 执行
    ret

/**
 * @brief 切换到调度线程，不保存旧的上下文
 * @param  _os              调度线程
 */
// void switch_os(context_t *_os);
.globl switch_os
.align 4
switch_os:
    // 恢复 _new
    context_load a0
    // 跳转到 _new->ra 执行
    ret

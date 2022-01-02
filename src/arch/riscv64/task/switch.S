
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

#include "context_switch.S"

// void switch_context_init(context_t *_context);
.globl switch_context_init
.align 4
switch_context_init:
    sd_base ra, 0, a0
    addi a0, a0, 8
    callee_regs_save a0
    addi a0, a0, CALLEE_SIZE
    csrr t6, satp
    sd_base t6, 0, a0
    addi a0, a0, 8
    csrr t6, sepc
    sd_base t6, 0, a0
    addi a0, a0, 8
    ld_base t6, 0, a0
    csrw sscratch, t6
    ret

// 进程上下文的切换
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

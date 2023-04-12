
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

/**
 * @brief 初始化第一个进程的上下文
 * @param  _context         第一个进程
 */
// void context_init(context_t *_context);
.global context_init
.align 4
context_init:
    ret

/**
 * @brief 上下文切换
 * @param  _old             当前进程
 * @param  _new             下一个进程
 */
// void switch_context(context_t *_old, context_t *_new);
/* void x86_64_context_switch(uint64_t *oldsp, uint64_t newsp) */
.global switch_context
.align 4
switch_context:
    /* save the old context and restore the new */
    // pushf
    // pushq %rbx
    // pushq %rbp
    // pushq %r12
    // pushq %r13
    // pushq %r14
    // pushq %r15

    // movq %rsp,(%rdi)
    // movq %rsi,%rsp

    // popq %r15
    // popq %r14
    // popq %r13
    // popq %r12
    // popq %rbp
    // popq %rbx
    // popf
    // retq

/**
 * @brief 切换到调度线程，不保存旧的上下文
 * @param  _os              调度线程
 */
// void switch_os(context_t *_os);
.global switch_os
.align 4
switch_os:
    ret

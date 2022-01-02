
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
    ret

// 进程上下文的切换
// void switch_context(context_t *_old, context_t *_new);
.globl switch_context
.align 4
//switch_context:
    // 保存 _old
    //context_save a0
    // 恢复 _new
    //context_load a1
    // 跳转到 _new->ra 执行
    //ret


/* void x86_64_context_switch(uint64_t *oldsp, uint64_t newsp) */
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

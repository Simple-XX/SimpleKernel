/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "macro.S"

.section .text
.global switch_to
.type switch_to, @function

/**
 * @brief 线程上下文切换
 *
 * void switch_to(CalleeSavedContext* prev, CalleeSavedContext* next);
 *
 * @param prev x0: 当前线程上下文结构体的地址
 * @param next x1: 下一个线程上下文结构体的地址
 */
switch_to:
    // 保存 Callee-saved 寄存器到 prev (x0) 指向的结构体
    SaveCalleeSavedContext x0

    // 从 next (x1) 指向的结构体恢复 Callee-saved 寄存器
    RestoreCalleeSavedContext x1

    ret

.global kernel_thread_entry
.type kernel_thread_entry, @function
.extern kernel_thread_bootstrap
kernel_thread_entry:
    // 上下文恢复时：
    // x30 (lr) = kernel_thread_entry
    // x19 = 真正的入口函数 entry
    // x20 = 参数 arg

    mov x0, x19
    mov x1, x20
    bl kernel_thread_bootstrap

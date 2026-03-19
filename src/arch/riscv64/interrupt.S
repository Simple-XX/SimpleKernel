/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "macro.S"

.section .text

.global trap_return
.type trap_return, @function

.global trap_entry
.type trap_entry, @function

.extern HandleTrap

.align 4

/**
 * @brief Trap 处理入口函数
 *
 * @details 当发生中断或异常时，CPU 跳转到此地址执行。
 * 主要功能：
 * 1. 保存当前 CPU 上下文 (SaveTrapContext)。
 * 2. 将当前栈指针 (sp) 作为参数传递给 HandleTrap。
 * 3. 调用 C++ 实现的 Trap 处理函数 HandleTrap。
 * 4. 恢复 CPU 上下文 (RestoreTrapContext)。
 * 5. 执行 sret 指令返回。
 */
trap_entry:
    // 检查是从用户态还是内核态进入
    // 如果是从用户态进入，sscratch 保存的是内核栈地址
    // 如果是从内核态进入，sscratch 为 0
    csrrw sp, sscratch, sp
    bnez sp, .Ltrap_from_user

    // 从内核态进入，恢复 sp，此时 sscratch 变回 0
    csrrw sp, sscratch, sp
    j .Lsave_context

.Ltrap_from_user:
    // 从用户态进入，此时 sp 为内核栈，sscratch 为用户栈 sp

.Lsave_context:
    // 保存 Trap 上下文
    SaveTrapContext

    // 检查 sscratch，如果非 0 说明是从用户态进入，Context 中的 sp (offset 1) 保存的是错误的（内核栈地址）
    // 需要将 sscratch（用户 sp）保存到 Context 中
    csrr t0, sscratch
    beqz t0, .Lcall_handler

    // 覆盖 Context 中的 sp
    sd t0, 8(sp)
    // 将 sscratch 清零，保证在内核态执行时 sscratch 为 0
    csrw sscratch, x0

.Lcall_handler:
    // 将栈指针 sp (指向 TrapContext) 作为参数传递给 HandleTrap
    mv a0, sp
    // 调用 C++ 中的 Trap 处理函数
    call HandleTrap

trap_return:
    // 1. kernel_thread_entry 跳转到这里时，a0 是参数 (Context*)
    // 2. 将 a0 赋值给 sp，切换到 Trap 上下文所在的栈位置
    mv sp, a0

    // 恢复 Trap 上下文
    // 检查 sstatus.SPP (offset 31) 判断返回到用户态还是内核态
    ld t0, 31*8(sp)
    andi t0, t0, 0x100
    bnez t0, .Lret_to_kernel

.Lret_to_user:
    // 返回用户态
    // 1. 取出用户栈 sp 到 t0
    ld t0, 8(sp)
    // 2. 将用户栈 sp 保存到 sscratch
    csrw sscratch, t0
    // 3. 恢复通用寄存器 (sp 此时指向内核栈顶)
    RestoreTrapContext
    // 4. 交换 sp and sscratch
    // 此时 sp = 用户栈, sscratch = 内核栈顶
    csrrw sp, sscratch, sp
    sret

.Lret_to_kernel:
    // 返回内核态
    RestoreTrapContext
    // 从 S 模式异常中返回
    sret

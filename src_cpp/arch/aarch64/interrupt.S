/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "macro.S"

.section .text

.global vector_table
.balign 0x800
vector_table:
// Current EL with SP0
.balign 0x80
    b sync_current_el_sp0
.balign 0x80
    b irq_current_el_sp0
.balign 0x80
    b fiq_current_el_sp0
.balign 0x80
    b error_current_el_sp0

// Current EL with SPx
.balign 0x80
    b sync_current_el_spx
.balign 0x80
    b irq_current_el_spx
.balign 0x80
    b fiq_current_el_spx
.balign 0x80
    b error_current_el_spx

// Lower EL using AArch64
.balign 0x80
    b sync_lower_el_aarch64
.balign 0x80
    b irq_lower_el_aarch64
.balign 0x80
    b fiq_lower_el_aarch64
.balign 0x80
    b error_lower_el_aarch64

// Lower EL using AArch32
.balign 0x80
    b sync_lower_el_aarch32
.balign 0x80
    b irq_lower_el_aarch32
.balign 0x80
    b fiq_lower_el_aarch32
.balign 0x80
    b error_lower_el_aarch32

// Exception handlers
sync_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl sync_current_el_sp0_handler
    RestoreTrapContext
    eret

irq_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl irq_current_el_sp0_handler
    RestoreTrapContext
    eret

fiq_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl fiq_current_el_sp0_handler
    RestoreTrapContext
    eret

error_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl error_current_el_sp0_handler
    RestoreTrapContext
    eret

sync_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl sync_current_el_spx_handler
    RestoreTrapContext
    eret

irq_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl irq_current_el_spx_handler
    RestoreTrapContext
    eret

fiq_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl fiq_current_el_spx_handler
    RestoreTrapContext
    eret

error_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl error_current_el_spx_handler
    RestoreTrapContext
    eret

sync_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl sync_lower_el_aarch64_handler
    RestoreTrapContext
    eret

irq_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl irq_lower_el_aarch64_handler
    RestoreTrapContext
    eret

fiq_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl fiq_lower_el_aarch64_handler
    RestoreTrapContext
    eret

error_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl error_lower_el_aarch64_handler
    RestoreTrapContext
    eret

sync_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl sync_lower_el_aarch32_handler
    RestoreTrapContext
    eret

irq_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl irq_lower_el_aarch32_handler
    RestoreTrapContext
    eret

fiq_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl fiq_lower_el_aarch32_handler
    RestoreTrapContext
    eret

error_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl error_lower_el_aarch32_handler
    RestoreTrapContext
    eret

// trap_return: 用于从内核返回到用户态或内核态
// 参数：x0 = TrapContext*
.global trap_return
.type trap_return, @function
trap_return:
    // kernel_thread_entry 跳转到这里时，x0 是参数 (TrapContext*)
    // 将 x0 赋值给 sp，切换到 Trap 上下文所在的栈位置
    mov sp, x0

    // 恢复 Trap 上下文
    // 检查 spsr_el1.M[3:0] 判断返回到用户态还是内核态
    // spsr_el1 at offset 264
    ldr x9, [sp, #264]
    // 提取 M[3:0]
    and x9, x9, #0xF
    cbnz x9, .Lret_to_kernel

.Lret_to_user:
    // 返回用户态
    // ttbr0_el1 at offset 296
    ldr x9, [sp, #296]
    msr ttbr0_el1, x9
    isb

    // 恢复通用寄存器和系统寄存器
    RestoreTrapContext
    eret

.Lret_to_kernel:
    // 返回内核态
    RestoreTrapContext
    eret

/**
 * @copyright Copyright The SimpleKernel Contributors
 * @note 需要与 MRNIU/cpu_io/include/aarch64/context.hpp 中的结构体定义保持一致
 */

// 对齐到 16 字节
#define ALIGN_16(x) (((x) + 15) & ~15)

/**
 * @brief 寄存器长度，8 字节
 */
.equ kRegsBytes, 8

/**
 * @brief TrapContext 结构体大小
 */
.equ kTrapContextSize, 320

/**
 * @brief CalleeSavedContext 结构体大小
 */
.equ kCalleeSavedContextSize, 112

/**
 * @brief 保存中断/异常上下文 (TrapContext)
 *
 * 保存所有寄存器到栈上
 * 偏移 0-247: x0-x30
 * 偏移 248-255: _padding0
 * 偏移 256-271: elr_el1, spsr_el1
 * 偏移 272-279: esr_el1
 * 偏移 280-295: sp_el0, tpidr_el0
 * 偏移 296-303: ttbr0_el1
 * 偏移 304-319: sp_el1, tpidr_el1
 *
 * @note 此宏会自动分配栈空间 (sub sp, sp, #320)。
 */
.macro SaveTrapContext
    // 分配栈空间 (320 字节)
    sub sp, sp, #kTrapContextSize

    // 保存通用寄存器 x0-x30 (偏移 0-247)
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #240]
    // 偏移 248-255: _padding0 (不需要显式保存)

    // 保存系统寄存器 (偏移 256-319)
    mrs x9, elr_el1
    mrs x10, spsr_el1
    stp x9, x10, [sp, #256]
    mrs x9, esr_el1
    str x9, [sp, #272]

    mrs x9, sp_el0
    mrs x10, tpidr_el0
    stp x9, x10, [sp, #280]
    mrs x9, ttbr0_el1
    str x9, [sp, #296]

    // sp_el1: 保存分配前的栈指针
    add x9, sp, #kTrapContextSize
    mrs x10, tpidr_el1
    stp x9, x10, [sp, #304]
.endm

/**
 * @brief 恢复中断/异常上下文 (TrapContext)
 *
 * 从栈上恢复所有寄存器，严格按照 context.hpp 中 TrapContext 的布局。
 *
 * @note 此宏会恢复 sp，等于隐式释放栈空间。
 */
.macro RestoreTrapContext
    // 恢复系统寄存器 (偏移 256-319)
    ldp x9, x10, [sp, #256]
    msr elr_el1, x9
    msr spsr_el1, x10
    // 注意：不恢复 esr_el1，它是硬件在异常时自动设置的只读寄存器

    ldp x9, x10, [sp, #280]
    ldr x11, [sp, #296]
    msr sp_el0, x9
    msr tpidr_el0, x10
    msr ttbr0_el1, x11

    ldp x9, x10, [sp, #304]
    // sp_el1 最后恢复
    msr tpidr_el1, x10

    // 恢复通用寄存器 x0-x30 (偏移 0-247)
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #240]

    // 恢复 sp (释放栈空间)
    add sp, sp, #kTrapContextSize
.endm

/**
 * @brief 保存 Callee-saved 寄存器到指定地址 (CalleeSavedContext)
 *
 * 按照 context.hpp 中 CalleeSavedContext 的布局：
 * 偏移 0-95: x19-x30 (12 regs)
 * 偏移 96-111: sp, pc (2 regs)
 *
 * @param base_reg 基地址寄存器 (保存的目标地址)
 */
.macro SaveCalleeSavedContext base_reg
    // 保存 x19-x30 (偏移 0-95)
    stp x19, x20, [\base_reg, #0]
    stp x21, x22, [\base_reg, #16]
    stp x23, x24, [\base_reg, #32]
    stp x25, x26, [\base_reg, #48]
    stp x27, x28, [\base_reg, #64]
    stp x29, x30, [\base_reg, #80]

    // 保存 sp 和 pc (偏移 96-111)
    mov x9, sp
    mov x10, x30  // pc: 保存返回地址（Link Register）
    stp x9, x10, [\base_reg, #96]
.endm

/**
 * @brief 从指定地址恢复 Callee-saved 寄存器 (CalleeSavedContext)
 *
 * 按照 context.hpp 中 CalleeSavedContext 的布局恢复寄存器。
 *
 * @param base_reg 基地址寄存器 (恢复的源地址)
 */
.macro RestoreCalleeSavedContext base_reg
    // 恢复 sp 和 pc (偏移 96-111)
    ldp x9, x10, [\base_reg, #96]
    mov sp, x9

    // 恢复 x19-x30 (偏移 0-95)
    ldp x19, x20, [\base_reg, #0]
    ldp x21, x22, [\base_reg, #16]
    ldp x23, x24, [\base_reg, #32]
    ldp x25, x26, [\base_reg, #48]
    ldp x27, x28, [\base_reg, #64]
    ldp x29, x30, [\base_reg, #80]

    // 跳转到保存的 pc
    br x10
.endm

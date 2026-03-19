/**
 * @copyright Copyright The SimpleKernel Contributors
 * @note 需要与 MRNIU/cpu_io/include/riscv64/context.hpp 中的结构体定义保持一致
 */

// 对齐到 16 字节
#define ALIGN_16(x) (((x) + 15) & ~15)

/**
 * @brief 寄存器长度，8 字节
 */
.equ kRegsBytes, 8

/**
 * @brief 中断/异常上下文切换所需保存的寄存器数量
 *
 * 31 个通用寄存器(x1-x31) + 4 个 CSR + 1 个 padding
 * 总计: 31 + 4 + 1 = 36
 */
.equ kTrapContextRegsCount, 36

/**
 * @brief 中断/异常上下文切换所需保存的寄存器大小
 * 36 个寄存器 * 8 字节 = 288 字节（已对齐到 16 字节）
 */
.equ kTrapContextSize, ALIGN_16(kTrapContextRegsCount * kRegsBytes)

/**
 * @brief 函数调用上下文切换所需保存的寄存器数量 (Callee-saved)
 * ra, sp, s0-s11 = 14 regs
 */
.equ kCalleeSavedContextRegsCount, 14

/**
 * @brief 函数调用上下文切换所需保存的寄存器大小 (Callee-saved)
 * 14 * 8 = 112 字节（已对齐到 16 字节）
 */
.equ kCalleeSavedContextSize, ALIGN_16(kCalleeSavedContextRegsCount * kRegsBytes)

/**
 * @brief 将寄存器 a 保存在 c 偏移 b 的位置
 * @param a 源寄存器
 * @param b 偏移量 (乘以 kRegsBytes)
 * @param c 基地址寄存器
 */
.macro SdBase a, b, c
    sd \a, ((\b) * kRegsBytes)(\c)
.endm

/**
 * @brief 从 c 的偏移 b 处获取数据并赋值给寄存器 a
 * @param a 目标寄存器
 * @param b 偏移量 (乘以 kRegsBytes)
 * @param c 基地址寄存器
 */
.macro LdBase a, b, c
    ld \a, ((\b) * kRegsBytes)(\c)
.endm

/**
 * @brief 保存中断/异常上下文 (TrapContext)
 *
 * 保存所有通用寄存器 (x1-x31) 和必要的 CSR 到栈上。
 *
 * @note 此宏会自动分配栈空间 (addi sp, sp, -kTrapContextSize)。
 * @note 保存的 sp 是分配空间之前的 sp。
 * @note 如果是从用户态陷入，通常需要手动将 sscratch (用户 sp) 覆盖到 offset 1。
 */
.macro SaveTrapContext
    addi sp, sp, -kTrapContextSize

    // 保存通用寄存器 (除了 sp)
    SdBase ra, 0, sp
    // sp 需要特殊处理，这里先跳过，最后计算保存
    SdBase gp, 2, sp
    SdBase tp, 3, sp
    SdBase t0, 4, sp
    SdBase t1, 5, sp
    SdBase t2, 6, sp
    SdBase s0, 7, sp
    SdBase s1, 8, sp
    SdBase a0, 9, sp
    SdBase a1, 10, sp
    SdBase a2, 11, sp
    SdBase a3, 12, sp
    SdBase a4, 13, sp
    SdBase a5, 14, sp
    SdBase a6, 15, sp
    SdBase a7, 16, sp
    SdBase s2, 17, sp
    SdBase s3, 18, sp
    SdBase s4, 19, sp
    SdBase s5, 20, sp
    SdBase s6, 21, sp
    SdBase s7, 22, sp
    SdBase s8, 23, sp
    SdBase s9, 24, sp
    SdBase s10, 25, sp
    SdBase s11, 26, sp
    SdBase t3, 27, sp
    SdBase t4, 28, sp
    SdBase t5, 29, sp
    SdBase t6, 30, sp

    // 保存原来的 sp 到 offset 1
    addi t0, sp, kTrapContextSize
    SdBase t0, 1, sp

    // 保存 CSRs (使用 t0 作为临时寄存器，t0 原值已 preservation at offset 4)
    csrr t0, sstatus
    SdBase t0, 31, sp
    csrr t0, sepc
    SdBase t0, 32, sp
    csrr t0, stval
    SdBase t0, 33, sp
    csrr t0, scause
    SdBase t0, 34, sp
.endm

/**
 * @brief 恢复中断/异常上下文 (TrapContext)
 *
 * 从栈上恢复所有通用寄存器 (x1-x31) 和 CSR。
 *
 * @note 此宏会恢复 sp，等于隐式释放栈空间 (若 TrapContext 中的 sp 是分配前的栈顶)。
 */
.macro RestoreTrapContext
    // 恢复 CSRs (使用 t0 作为临时寄存器)
    LdBase t0, 31, sp
    csrw sstatus, t0
    LdBase t0, 32, sp
    csrw sepc, t0
    LdBase t0, 33, sp
    csrw stval, t0
    LdBase t0, 34, sp
    csrw scause, t0

    // 恢复通用寄存器 (除了 sp 和 t0)
    LdBase ra, 0, sp
    LdBase gp, 2, sp
    LdBase tp, 3, sp
    // t0 (4) 最后恢复
    LdBase t1, 5, sp
    LdBase t2, 6, sp
    LdBase s0, 7, sp
    LdBase s1, 8, sp
    LdBase a0, 9, sp
    LdBase a1, 10, sp
    LdBase a2, 11, sp
    LdBase a3, 12, sp
    LdBase a4, 13, sp
    LdBase a5, 14, sp
    LdBase a6, 15, sp
    LdBase a7, 16, sp
    LdBase s2, 17, sp
    LdBase s3, 18, sp
    LdBase s4, 19, sp
    LdBase s5, 20, sp
    LdBase s6, 21, sp
    LdBase s7, 22, sp
    LdBase s8, 23, sp
    LdBase s9, 24, sp
    LdBase s10, 25, sp
    LdBase s11, 26, sp
    LdBase t3, 27, sp
    LdBase t4, 28, sp
    LdBase t5, 29, sp
    LdBase t6, 30, sp

    // 恢复 t0
    LdBase t0, 4, sp

    // 恢复 sp
    addi sp, sp, kTrapContextSize
.endm

/**
 * @brief 保存 Callee-saved 寄存器到指定地址
 * 保存 ra, sp, s0-s11
 *
 * @param base_reg 基地址寄存器 (保存的目标地址)
 */
.macro SaveCalleeSavedContext base_reg
    SdBase ra, 0, \base_reg
    SdBase sp, 1, \base_reg
    SdBase s0, 2, \base_reg
    SdBase s1, 3, \base_reg
    SdBase s2, 4, \base_reg
    SdBase s3, 5, \base_reg
    SdBase s4, 6, \base_reg
    SdBase s5, 7, \base_reg
    SdBase s6, 8, \base_reg
    SdBase s7, 9, \base_reg
    SdBase s8, 10, \base_reg
    SdBase s9, 11, \base_reg
    SdBase s10, 12, \base_reg
    SdBase s11, 13, \base_reg
.endm

/**
 * @brief 从指定地址恢复 Callee-saved 寄存器
 * 恢复 ra, sp, s0-s11
 *
 * @param base_reg 基地址寄存器 (恢复的源地址)
 */
.macro RestoreCalleeSavedContext base_reg
    LdBase ra, 0, \base_reg
    LdBase sp, 1, \base_reg
    LdBase s0, 2, \base_reg
    LdBase s1, 3, \base_reg
    LdBase s2, 4, \base_reg
    LdBase s3, 5, \base_reg
    LdBase s4, 6, \base_reg
    LdBase s5, 7, \base_reg
    LdBase s6, 8, \base_reg
    LdBase s7, 9, \base_reg
    LdBase s8, 10, \base_reg
    LdBase s9, 11, \base_reg
    LdBase s10, 12, \base_reg
    LdBase s11, 13, \base_reg
.endm

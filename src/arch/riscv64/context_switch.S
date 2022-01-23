
/**
 * @file context_switch.S
 * @brief 上下文切换
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

// 寄存器长度，8 字节
.equ REG_BYTES, 8
// 保存所有寄存器
.equ CALLEE_SAVE_REGS, 16
// 需要的大小
.equ CALLEE_SIZE, (CALLEE_SAVE_REGS * REG_BYTES)

// 一些宏
.macro sd_base a, b, c
sd \a, ((\b)*REG_BYTES)(\c)
.endm

.macro ld_base a, b, c
ld \a, ((\b)*REG_BYTES)(\c)
.endm

.macro fsd_base a, b, c
fsd \a, ((\b)*REG_BYTES)(\c)
.endm

.macro fld_base a, b, c
fld \a, ((\b)*REG_BYTES)(\c)
.endm

// 保存
.macro callee_regs_save base
    sd_base ra, 0, \base
    sd_base a0, 1, \base
    sd_base a1, 2, \base
    sd_base a2, 3, \base
    sd_base a3, 4, \base
    sd_base a4, 5, \base
    sd_base a5, 6, \base
    sd_base a6, 7, \base
    sd_base a7, 8, \base
    sd_base t0, 9, \base
    sd_base t1, 10, \base
    sd_base t2, 11, \base
    sd_base t3, 12, \base
    sd_base t4, 13, \base
    sd_base t5, 14, \base
    sd_base t6, 15, \base
.endm

// 恢复
.macro callee_regs_load base
    ld_base ra, 0, \base
    ld_base a0, 1, \base
    ld_base a1, 2, \base
    ld_base a2, 3, \base
    ld_base a3, 4, \base
    ld_base a4, 5, \base
    ld_base a5, 6, \base
    ld_base a6, 7, \base
    ld_base a7, 8, \base
    ld_base t0, 9, \base
    ld_base t1, 10, \base
    ld_base t2, 11, \base
    ld_base t3, 12, \base
    ld_base t4, 13, \base
    ld_base t5, 14, \base
    ld_base t6, 15, \base
.endm

// 保存 context_t
.macro context_save base
    callee_regs_save \base
.endm

// 恢复 context_t
.macro context_load base
    callee_regs_load \base
.endm

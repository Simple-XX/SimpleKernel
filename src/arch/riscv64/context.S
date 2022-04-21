
/**
 * @file context.S
 * @brief 上下文保存
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
// 所有寄存器数量
.equ ALL_REGS, 70
// 被调用者保存寄存器数量
.equ CALLEE_SAVE_REGS, 25
// 调用者保存寄存器数量
.equ CALLER_SAVE_REGS, 35
// 保存所有寄存器需要的大小
.equ ALL_SIZE,    (ALL_REGS * REG_BYTES)
// 保存被调用者保存寄存器需要的大小
.equ CALLEE_SIZE, (CALLEE_SAVE_REGS * REG_BYTES)
// 保存调用者保存寄存器需要的大小
.equ CALLER_SIZE, (CALLER_SAVE_REGS * REG_BYTES)

// 将寄存器 a 保存在 c 偏移 b 的位置
.macro sd_base a, b, c
sd \a, ((\b) * REG_BYTES)(\c)
.endm

// 从 c 的偏移 b 处获取数据并赋值给寄存器 a
.macro ld_base a, b, c
ld \a, ((\b) * REG_BYTES)(\c)
.endm

// 将 float 寄存器 a 保存在 c 偏移 b 的位置
.macro fsd_base a, b, c
fsd \a, ((\b) * REG_BYTES)(\c)
.endm

// 从 c 的偏移 b 处获取数据并赋值给 float 寄存器 a
.macro fld_base a, b, c
fld \a, ((\b) * REG_BYTES)(\c)
.endm

/**
 * @brief 保存所有寄存器
 * @param  _base           要保存到的基地址
 */
.macro all_regs_save _base
    sd_base  zero, 0,  \_base
    sd_base  ra,   1,  \_base
    sd_base  sp,   2,  \_base
    sd_base  gp,   3,  \_base
    sd_base  tp,   4,  \_base
    sd_base  t0,   5,  \_base
    sd_base  t1,   6,  \_base
    sd_base  t2,   7,  \_base
    sd_base  s0,   8,  \_base
    sd_base  s1,   9,  \_base
    sd_base  a0,   10, \_base
    sd_base  a1,   11, \_base
    sd_base  a2,   12, \_base
    sd_base  a3,   13, \_base
    sd_base  a4,   14, \_base
    sd_base  a5,   15, \_base
    sd_base  a6,   16, \_base
    sd_base  a7,   17, \_base
    sd_base  s2,   18, \_base
    sd_base  s3,   19, \_base
    sd_base  s4,   20, \_base
    sd_base  s5,   21, \_base
    sd_base  s6,   22, \_base
    sd_base  s7,   23, \_base
    sd_base  s8,   24, \_base
    sd_base  s9,   25, \_base
    sd_base  s10,  26, \_base
    sd_base  s11,  27, \_base
    sd_base  t3,   28, \_base
    sd_base  t4,   29, \_base
    sd_base  t5,   30, \_base
    sd_base  t6,   31, \_base

    fsd_base ft0,  32, \_base
    fsd_base ft1,  33, \_base
    fsd_base ft2,  34, \_base
    fsd_base ft3,  35, \_base
    fsd_base ft4,  36, \_base
    fsd_base ft5,  37, \_base
    fsd_base ft6,  38, \_base
    fsd_base ft7,  39, \_base
    fsd_base fs0,  40, \_base
    fsd_base fs1,  41, \_base
    fsd_base fa0,  42, \_base
    fsd_base fa1,  43, \_base
    fsd_base fa2,  44, \_base
    fsd_base fa3,  45, \_base
    fsd_base fa4,  46, \_base
    fsd_base fa5,  47, \_base
    fsd_base fa6,  48, \_base
    fsd_base fa7,  49, \_base
    fsd_base fs2,  50, \_base
    fsd_base fs3,  51, \_base
    fsd_base fs4,  52, \_base
    fsd_base fs5,  53, \_base
    fsd_base fs6,  54, \_base
    fsd_base fs7,  55, \_base
    fsd_base fs8,  56, \_base
    fsd_base fs9,  57, \_base
    fsd_base fs10, 58, \_base
    fsd_base fs11, 59, \_base
    fsd_base ft8,  60, \_base
    fsd_base ft9,  61, \_base
    fsd_base ft10, 62, \_base
    fsd_base ft11, 63, \_base

    csrr     t0,   sepc
    sd_base  t0,   64, \_base
    csrr     t0,   stval
    sd_base  t0,   65, \_base
    csrr     t0,   scause
    sd_base  t0,   66, \_base
    csrr     t0,   sie
    sd_base  t0,   67, \_base
    csrr     t0,   sstatus
    sd_base  t0,   68, \_base
    csrr     t0,   sscratch
    sd_base  t0,   69, \_base
.endm

/**
 * @brief 恢复所有寄存器
 * @param  _base           要恢复数据的基地址
 */
.macro all_regs_load _base
    ld_base  t0,   64, \_base
    csrw     sepc,     t0
    ld_base  t0,   65, \_base
    csrw     stval,    t0
    ld_base  t0,   66, \_base
    csrw     scause,   t0
    ld_base  t0,   67, \_base
    csrw     sie,  t0
    ld_base  t0,   68, \_base
    csrw     sstatus,  t0
    ld_base  t0,   69, \_base
    csrw     sscratch, t0

    ld_base  zero, 0,  \_base
    ld_base  ra,   1,  \_base
    ld_base  sp,   2,  \_base
    ld_base  gp,   3,  \_base
    ld_base  tp,   4,  \_base
    ld_base  t0,   5,  \_base
    ld_base  t1,   6,  \_base
    ld_base  t2,   7,  \_base
    ld_base  s0,   8,  \_base
    ld_base  s1,   9,  \_base
    ld_base  a0,   10, \_base
    ld_base  a1,   11, \_base
    ld_base  a2,   12, \_base
    ld_base  a3,   13, \_base
    ld_base  a4,   14, \_base
    ld_base  a5,   15, \_base
    ld_base  a6,   16, \_base
    ld_base  a7,   17, \_base
    ld_base  s2,   18, \_base
    ld_base  s3,   19, \_base
    ld_base  s4,   20, \_base
    ld_base  s5,   21, \_base
    ld_base  s6,   22, \_base
    ld_base  s7,   23, \_base
    ld_base  s8,   24, \_base
    ld_base  s9,   25, \_base
    ld_base  s10,  26, \_base
    ld_base  s11,  27, \_base
    ld_base  t3,   28, \_base
    ld_base  t4,   29, \_base
    ld_base  t5,   30, \_base
    ld_base  t6,   31, \_base

    fld_base ft0,  32, \_base
    fld_base ft1,  33, \_base
    fld_base ft2,  34, \_base
    fld_base ft3,  35, \_base
    fld_base ft4,  36, \_base
    fld_base ft5,  37, \_base
    fld_base ft6,  38, \_base
    fld_base ft7,  39, \_base
    fld_base fs0,  40, \_base
    fld_base fs1,  41, \_base
    fld_base fa0,  42, \_base
    fld_base fa1,  43, \_base
    fld_base fa2,  44, \_base
    fld_base fa3,  45, \_base
    fld_base fa4,  46, \_base
    fld_base fa5,  47, \_base
    fld_base fa6,  48, \_base
    fld_base fa7,  49, \_base
    fld_base fs2,  50, \_base
    fld_base fs3,  51, \_base
    fld_base fs4,  52, \_base
    fld_base fs5,  53, \_base
    fld_base fs6,  54, \_base
    fld_base fs7,  55, \_base
    fld_base fs8,  56, \_base
    fld_base fs9,  57, \_base
    fld_base fs10, 58, \_base
    fld_base fs11, 59, \_base
    fld_base ft8,  60, \_base
    fld_base ft9,  61, \_base
    fld_base ft10, 62, \_base
    fld_base ft11, 63, \_base
.endm

/**
 * @brief 保存调用者保存寄存器
 * @param  _base           要保存到的基地址
 * @param  _off:           基地址的偏移
 */
.macro caller_regs_save _base, _off
    sd_base  ra,   0  + \_off, \_base
    sd_base  t0,   1  + \_off, \_base
    sd_base  t2,   2  + \_off, \_base
    sd_base  a0,   3  + \_off, \_base
    sd_base  a1,   4  + \_off, \_base
    sd_base  a2,   5  + \_off, \_base
    sd_base  a3,   6  + \_off, \_base
    sd_base  a4,   7  + \_off, \_base
    sd_base  a5,   8  + \_off, \_base
    sd_base  a6,   9  + \_off, \_base
    sd_base  a7,   10 + \_off, \_base
    sd_base  t3,   11 + \_off, \_base
    sd_base  t4,   12 + \_off, \_base
    sd_base  t5,   13 + \_off, \_base
    sd_base  t6,   14 + \_off, \_base
    fsd_base ft0,  15 + \_off, \_base
    fsd_base ft1,  16 + \_off, \_base
    fsd_base ft2,  17 + \_off, \_base
    fsd_base ft3,  18 + \_off, \_base
    fsd_base ft4,  19 + \_off, \_base
    fsd_base ft5,  20 + \_off, \_base
    fsd_base ft6,  21 + \_off, \_base
    fsd_base ft7,  22 + \_off, \_base
    fsd_base fa0,  23 + \_off, \_base
    fsd_base fa1,  24 + \_off, \_base
    fsd_base fa2,  25 + \_off, \_base
    fsd_base fa3,  26 + \_off, \_base
    fsd_base fa4,  27 + \_off, \_base
    fsd_base fa5,  28 + \_off, \_base
    fsd_base fa6,  29 + \_off, \_base
    fsd_base fa7,  30 + \_off, \_base
    fsd_base ft8,  31 + \_off, \_base
    fsd_base ft9,  32 + \_off, \_base
    fsd_base ft10, 33 + \_off, \_base
    fsd_base ft11, 34 + \_off, \_base
.endm

/**
 * @brief 恢复调用者保存寄存器
 * @param  _base           要恢复的基地址
 * @param  _off:           基地址的偏移
 */
.macro caller_regs_load _base, _off
    ld_base  ra,   0  + \_off, \_base
    ld_base  t0,   1  + \_off, \_base
    ld_base  t2,   2  + \_off, \_base
    ld_base  a0,   3  + \_off, \_base
    ld_base  a1,   4  + \_off, \_base
    ld_base  a2,   5  + \_off, \_base
    ld_base  a3,   6  + \_off, \_base
    ld_base  a4,   7  + \_off, \_base
    ld_base  a5,   8  + \_off, \_base
    ld_base  a6,   9  + \_off, \_base
    ld_base  a7,   10 + \_off, \_base
    ld_base  t3,   11 + \_off, \_base
    ld_base  t4,   12 + \_off, \_base
    ld_base  t5,   13 + \_off, \_base
    ld_base  t6,   14 + \_off, \_base
    fld_base ft0,  15 + \_off, \_base
    fld_base ft1,  16 + \_off, \_base
    fld_base ft2,  17 + \_off, \_base
    fld_base ft3,  18 + \_off, \_base
    fld_base ft4,  19 + \_off, \_base
    fld_base ft5,  20 + \_off, \_base
    fld_base ft6,  21 + \_off, \_base
    fld_base ft7,  22 + \_off, \_base
    fld_base fa0,  23 + \_off, \_base
    fld_base fa1,  24 + \_off, \_base
    fld_base fa2,  25 + \_off, \_base
    fld_base fa3,  26 + \_off, \_base
    fld_base fa4,  27 + \_off, \_base
    fld_base fa5,  28 + \_off, \_base
    fld_base fa6,  29 + \_off, \_base
    fld_base fa7,  30 + \_off, \_base
    fld_base ft8,  31 + \_off, \_base
    fld_base ft9,  32 + \_off, \_base
    fld_base ft10, 33 + \_off, \_base
    fld_base ft11, 34 + \_off, \_base
.endm

/**
 * @brief 保存被调用者保存寄存器
 * @param  _base           要保存的基地址
 * @param  _off:           基地址的偏移
 */
.macro callee_regs_save _base, _off
    sd_base  sp,   0  + \_off, \_base
    sd_base  s0,   1  + \_off, \_base
    sd_base  s1,   2  + \_off, \_base
    sd_base  s2,   3  + \_off, \_base
    sd_base  s3,   4  + \_off, \_base
    sd_base  s4,   5  + \_off, \_base
    sd_base  s5,   6  + \_off, \_base
    sd_base  s6,   7  + \_off, \_base
    sd_base  s7,   8  + \_off, \_base
    sd_base  s8,   9  + \_off, \_base
    sd_base  s9,   10 + \_off, \_base
    sd_base  s10,  11 + \_off, \_base
    sd_base  s11,  12 + \_off, \_base
    fsd_base fs0,  13 + \_off, \_base
    fsd_base fs1,  14 + \_off, \_base
    fsd_base fs2,  15 + \_off, \_base
    fsd_base fs3,  16 + \_off, \_base
    fsd_base fs4,  17 + \_off, \_base
    fsd_base fs5,  18 + \_off, \_base
    fsd_base fs6,  19 + \_off, \_base
    fsd_base fs7,  20 + \_off, \_base
    fsd_base fs8,  21 + \_off, \_base
    fsd_base fs9,  22 + \_off, \_base
    fsd_base fs10, 23 + \_off, \_base
    fsd_base fs11, 24 + \_off, \_base
.endm

/**
 * @brief 恢复被调用者保存寄存器
 * @param  _base           要恢复数据的基地址
 * @param  _off:           基地址的偏移
 */
.macro callee_regs_load _base, _off
    ld_base  sp,   0  + \_off, \_base
    ld_base  s0,   1  + \_off, \_base
    ld_base  s1,   2  + \_off, \_base
    ld_base  s2,   3  + \_off, \_base
    ld_base  s3,   4  + \_off, \_base
    ld_base  s4,   5  + \_off, \_base
    ld_base  s5,   6  + \_off, \_base
    ld_base  s6,   7  + \_off, \_base
    ld_base  s7,   8  + \_off, \_base
    ld_base  s8,   9  + \_off, \_base
    ld_base  s9,   10 + \_off, \_base
    ld_base  s10,  11 + \_off, \_base
    ld_base  s11,  12 + \_off, \_base
    fld_base fs0,  13 + \_off, \_base
    fld_base fs1,  14 + \_off, \_base
    fld_base fs2,  15 + \_off, \_base
    fld_base fs3,  16 + \_off, \_base
    fld_base fs4,  17 + \_off, \_base
    fld_base fs5,  18 + \_off, \_base
    fld_base fs6,  19 + \_off, \_base
    fld_base fs7,  20 + \_off, \_base
    fld_base fs8,  21 + \_off, \_base
    fld_base fs9,  22 + \_off, \_base
    fld_base fs10, 23 + \_off, \_base
    fld_base fs11, 24 + \_off, \_base
.endm

/**
 * @brief 保存 context_t
 * @param  _base           要保存到的基地址
 */
.macro context_save _base
    // @todo 临时设置为 tp
    sd_base          tp,       0,                    \_base
    sd_base          ra,       1,                    \_base
    callee_regs_save \_base,   2

    csrr             t6,       satp
    sd_base          t6,       2 + CALLEE_SAVE_REGS, \_base

    csrr             t6,       sepc
    sd_base          t6,       3 + CALLEE_SAVE_REGS, \_base

    csrr             t6,       sstatus
    sd_base          t6,       4 + CALLEE_SAVE_REGS, \_base

    csrr             t6,       sie
    sd_base          t6,       5 + CALLEE_SAVE_REGS, \_base

    csrr             t6,       sip
    sd_base          t6,       6 + CALLEE_SAVE_REGS, \_base

    csrr             t6,       sscratch
    sd_base          t6,       7 + CALLEE_SAVE_REGS, \_base
.endm

/**
 * @brief 恢复 context_t
 * @param  _base           要恢复数据的基地址
 */
.macro context_load _base
    // @todo 临时设置为 tp
    ld_base          tp,       0,                    \_base
    ld_base          ra,       1,                    \_base
    callee_regs_load \_base,   2

    ld_base          t6,       2 + CALLEE_SAVE_REGS, \_base
    csrw             satp,     t6

    ld_base          t6,       3 + CALLEE_SAVE_REGS, \_base
    csrw             sepc,     t6

    ld_base          t6,       4 + CALLEE_SAVE_REGS, \_base
    csrw             sstatus,  t6

    ld_base          t6,       5 + CALLEE_SAVE_REGS, \_base
    csrw             sie,      t6

    ld_base          t6,       6 + CALLEE_SAVE_REGS, \_base
    csrw             sip,      t6

    ld_base          t6,       7 + CALLEE_SAVE_REGS, \_base
    csrw             sscratch, t6
.endm

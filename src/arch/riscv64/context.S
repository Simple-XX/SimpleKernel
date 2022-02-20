
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
.equ ALL_REGS, 69
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

// 保存所有寄存器
.macro all_regs_save base
    // 在栈上留出保存寄存器的空间
    addi sp, sp, -ALL_SIZE

    sd_base  zero, 0,  \base
    sd_base  ra,   1,  \base
    sd_base  sp,   2,  \base
    sd_base  gp,   3,  \base
    sd_base  tp,   4,  \base
    sd_base  t0,   5,  \base
    sd_base  t1,   6,  \base
    sd_base  t2,   7,  \base
    sd_base  s0,   8,  \base
    sd_base  s1,   9,  \base
    sd_base  a0,   10, \base
    sd_base  a1,   11, \base
    sd_base  a2,   12, \base
    sd_base  a3,   13, \base
    sd_base  a4,   14, \base
    sd_base  a5,   15, \base
    sd_base  a6,   16, \base
    sd_base  a7,   17, \base
    sd_base  s2,   18, \base
    sd_base  s3,   19, \base
    sd_base  s4,   20, \base
    sd_base  s5,   21, \base
    sd_base  s6,   22, \base
    sd_base  s7,   23, \base
    sd_base  s8,   24, \base
    sd_base  s9,   25, \base
    sd_base  s10,  26, \base
    sd_base  s11,  27, \base
    sd_base  t3,   28, \base
    sd_base  t4,   29, \base
    sd_base  t5,   30, \base
    sd_base  t6,   31, \base

    fsd_base ft0,  32, \base
    fsd_base ft1,  33, \base
    fsd_base ft2,  34, \base
    fsd_base ft3,  35, \base
    fsd_base ft4,  36, \base
    fsd_base ft5,  37, \base
    fsd_base ft6,  38, \base
    fsd_base ft7,  39, \base
    fsd_base fs0,  40, \base
    fsd_base fs1,  41, \base
    fsd_base fa0,  42, \base
    fsd_base fa1,  43, \base
    fsd_base fa2,  44, \base
    fsd_base fa3,  45, \base
    fsd_base fa4,  46, \base
    fsd_base fa5,  47, \base
    fsd_base fa6,  48, \base
    fsd_base fa7,  49, \base
    fsd_base fs2,  50, \base
    fsd_base fs3,  51, \base
    fsd_base fs4,  52, \base
    fsd_base fs5,  53, \base
    fsd_base fs6,  54, \base
    fsd_base fs7,  55, \base
    fsd_base fs8,  56, \base
    fsd_base fs9,  57, \base
    fsd_base fs10, 58, \base
    fsd_base fs11, 59, \base
    fsd_base ft8,  60, \base
    fsd_base ft9,  61, \base
    fsd_base ft10, 62, \base
    fsd_base ft11, 63, \base

    csrr     t0,   sepc
    sd_base  t0,   64, \base
    csrr     t0,   stval
    sd_base  t0,   65, \base
    csrr     t0,   scause
    sd_base  t0,   66, \base
    csrr     t0,   sstatus
    sd_base  t0,   67, \base
    csrr     t0,   sscratch
    sd_base  t0,   68, \base
.endm

// 恢复所有寄存器
.macro all_regs_load base
    ld_base  t0,   64, \base
    csrw     sepc,     t0
    ld_base  t0,   65, \base
    csrw     stval,    t0
    ld_base  t0,   66, \base
    csrw     scause,   t0
    ld_base  t0,   67, \base
    csrw     sstatus,  t0
    ld_base  t0,   68, \base
    csrw     sscratch, t0

    ld_base  zero, 0,  \base
    ld_base  ra,   1,  \base
    ld_base  sp,   2,  \base
    ld_base  gp,   3,  \base
    ld_base  tp,   4,  \base
    ld_base  t0,   5,  \base
    ld_base  t1,   6,  \base
    ld_base  t2,   7,  \base
    ld_base  s0,   8,  \base
    ld_base  s1,   9,  \base
    ld_base  a0,   10, \base
    ld_base  a1,   11, \base
    ld_base  a2,   12, \base
    ld_base  a3,   13, \base
    ld_base  a4,   14, \base
    ld_base  a5,   15, \base
    ld_base  a6,   16, \base
    ld_base  a7,   17, \base
    ld_base  s2,   18, \base
    ld_base  s3,   19, \base
    ld_base  s4,   20, \base
    ld_base  s5,   21, \base
    ld_base  s6,   22, \base
    ld_base  s7,   23, \base
    ld_base  s8,   24, \base
    ld_base  s9,   25, \base
    ld_base  s10,  26, \base
    ld_base  s11,  27, \base
    ld_base  t3,   28, \base
    ld_base  t4,   29, \base
    ld_base  t5,   30, \base
    ld_base  t6,   31, \base

    fld_base ft0,  32, \base
    fld_base ft1,  33, \base
    fld_base ft2,  34, \base
    fld_base ft3,  35, \base
    fld_base ft4,  36, \base
    fld_base ft5,  37, \base
    fld_base ft6,  38, \base
    fld_base ft7,  39, \base
    fld_base fs0,  40, \base
    fld_base fs1,  41, \base
    fld_base fa0,  42, \base
    fld_base fa1,  43, \base
    fld_base fa2,  44, \base
    fld_base fa3,  45, \base
    fld_base fa4,  46, \base
    fld_base fa5,  47, \base
    fld_base fa6,  48, \base
    fld_base fa7,  49, \base
    fld_base fs2,  50, \base
    fld_base fs3,  51, \base
    fld_base fs4,  52, \base
    fld_base fs5,  53, \base
    fld_base fs6,  54, \base
    fld_base fs7,  55, \base
    fld_base fs8,  56, \base
    fld_base fs9,  57, \base
    fld_base fs10, 58, \base
    fld_base fs11, 59, \base
    fld_base ft8,  60, \base
    fld_base ft9,  61, \base
    fld_base ft10, 62, \base
    fld_base ft11, 63, \base

    // 手动释放栈上用于保存寄存器的空间
    addi     sp,   sp, ALL_SIZE
.endm

// 保存调用者保存寄存器
// base: 要保存到的基地址
// off: 基地址的偏移
.macro caller_regs_save base, off
    sd_base  ra,   0  + \off, \base
    sd_base  t0,   1  + \off, \base
    sd_base  t2,   2  + \off, \base
    sd_base  a0,   3  + \off, \base
    sd_base  a1,   4  + \off, \base
    sd_base  a2,   5  + \off, \base
    sd_base  a3,   6  + \off, \base
    sd_base  a4,   7  + \off, \base
    sd_base  a5,   8  + \off, \base
    sd_base  a6,   9  + \off, \base
    sd_base  a7,   10 + \off, \base
    sd_base  t3,   11 + \off, \base
    sd_base  t4,   12 + \off, \base
    sd_base  t5,   13 + \off, \base
    sd_base  t6,   14 + \off, \base
    fsd_base ft0,  15 + \off, \base
    fsd_base ft1,  16 + \off, \base
    fsd_base ft2,  17 + \off, \base
    fsd_base ft3,  18 + \off, \base
    fsd_base ft4,  19 + \off, \base
    fsd_base ft5,  20 + \off, \base
    fsd_base ft6,  21 + \off, \base
    fsd_base ft7,  22 + \off, \base
    fsd_base fa0,  23 + \off, \base
    fsd_base fa1,  24 + \off, \base
    fsd_base fa2,  25 + \off, \base
    fsd_base fa3,  26 + \off, \base
    fsd_base fa4,  27 + \off, \base
    fsd_base fa5,  28 + \off, \base
    fsd_base fa6,  29 + \off, \base
    fsd_base fa7,  30 + \off, \base
    fsd_base ft8,  31 + \off, \base
    fsd_base ft9,  32 + \off, \base
    fsd_base ft10, 33 + \off, \base
    fsd_base ft11, 34 + \off, \base
.endm

// 恢复调用者保存寄存器
// base: 要恢复的基地址
// off: 基地址的偏移
.macro caller_regs_load base, off
    ld_base  ra,   0  + \off, \base
    ld_base  t0,   1  + \off, \base
    ld_base  t2,   2  + \off, \base
    ld_base  a0,   3  + \off, \base
    ld_base  a1,   4  + \off, \base
    ld_base  a2,   5  + \off, \base
    ld_base  a3,   6  + \off, \base
    ld_base  a4,   7  + \off, \base
    ld_base  a5,   8  + \off, \base
    ld_base  a6,   9  + \off, \base
    ld_base  a7,   10 + \off, \base
    ld_base  t3,   11 + \off, \base
    ld_base  t4,   12 + \off, \base
    ld_base  t5,   13 + \off, \base
    ld_base  t6,   14 + \off, \base
    fld_base ft0,  15 + \off, \base
    fld_base ft1,  16 + \off, \base
    fld_base ft2,  17 + \off, \base
    fld_base ft3,  18 + \off, \base
    fld_base ft4,  19 + \off, \base
    fld_base ft5,  20 + \off, \base
    fld_base ft6,  21 + \off, \base
    fld_base ft7,  22 + \off, \base
    fld_base fa0,  23 + \off, \base
    fld_base fa1,  24 + \off, \base
    fld_base fa2,  25 + \off, \base
    fld_base fa3,  26 + \off, \base
    fld_base fa4,  27 + \off, \base
    fld_base fa5,  28 + \off, \base
    fld_base fa6,  29 + \off, \base
    fld_base fa7,  30 + \off, \base
    fld_base ft8,  31 + \off, \base
    fld_base ft9,  32 + \off, \base
    fld_base ft10, 33 + \off, \base
    fld_base ft11, 34 + \off, \base
.endm

// 保存被调用者保存寄存器
// base: 要保存的基地址
// off: 基地址的偏移
.macro callee_regs_save base, off
    sd_base  sp,   0  + \off, \base
    sd_base  s0,   1  + \off, \base
    sd_base  s1,   2  + \off, \base
    sd_base  s2,   3  + \off, \base
    sd_base  s3,   4  + \off, \base
    sd_base  s4,   5  + \off, \base
    sd_base  s5,   6  + \off, \base
    sd_base  s6,   7  + \off, \base
    sd_base  s7,   8  + \off, \base
    sd_base  s8,   9  + \off, \base
    sd_base  s9,   10 + \off, \base
    sd_base  s10,  11 + \off, \base
    sd_base  s11,  12 + \off, \base
    fsd_base fs0,  13 + \off, \base
    fsd_base fs1,  14 + \off, \base
    fsd_base fs2,  15 + \off, \base
    fsd_base fs3,  16 + \off, \base
    fsd_base fs4,  17 + \off, \base
    fsd_base fs5,  18 + \off, \base
    fsd_base fs6,  19 + \off, \base
    fsd_base fs7,  20 + \off, \base
    fsd_base fs8,  21 + \off, \base
    fsd_base fs9,  22 + \off, \base
    fsd_base fs10, 23 + \off, \base
    fsd_base fs11, 24 + \off, \base
.endm

// 恢复被调用者保存寄存器
// base: 要恢复的基地址
// off: 基地址的偏移
.macro callee_regs_load base, off
    ld_base  sp,   0  + \off, \base
    ld_base  s0,   1  + \off, \base
    ld_base  s1,   2  + \off, \base
    ld_base  s2,   3  + \off, \base
    ld_base  s3,   4  + \off, \base
    ld_base  s4,   5  + \off, \base
    ld_base  s5,   6  + \off, \base
    ld_base  s6,   7  + \off, \base
    ld_base  s7,   8  + \off, \base
    ld_base  s8,   9  + \off, \base
    ld_base  s9,   10 + \off, \base
    ld_base  s10,  11 + \off, \base
    ld_base  s11,  12 + \off, \base
    fld_base fs0,  13 + \off, \base
    fld_base fs1,  14 + \off, \base
    fld_base fs2,  15 + \off, \base
    fld_base fs3,  16 + \off, \base
    fld_base fs4,  17 + \off, \base
    fld_base fs5,  18 + \off, \base
    fld_base fs6,  19 + \off, \base
    fld_base fs7,  20 + \off, \base
    fld_base fs8,  21 + \off, \base
    fld_base fs9,  22 + \off, \base
    fld_base fs10, 23 + \off, \base
    fld_base fs11, 24 + \off, \base
.endm

// 保存 context_t
.macro context_save base
    // @todo 临时设置为 ra
    sd_base          ra,      0,                    \base
    sd_base          ra,      1,                    \base
    callee_regs_save \base,   2

    csrr             t6,       satp
    sd_base          t6,       2 + CALLEE_SAVE_REGS, \base

    csrr             t6,       sepc
    sd_base          t6,       3 + CALLEE_SAVE_REGS, \base

    csrr             t6,       sstatus
    sd_base          t6,       4 + CALLEE_SAVE_REGS, \base

    csrr             t6,       sie
    sd_base          t6,       5 + CALLEE_SAVE_REGS, \base

    csrr             t6,       sip
    sd_base          t6,       6 + CALLEE_SAVE_REGS, \base

    csrr             t6,       sscratch
    sd_base          t6,       7 + CALLEE_SAVE_REGS, \base
.endm

// 恢复 context_t
.macro context_load base
    ld_base          ra,       0,                    \base
    ld_base          ra,       1,                    \base
    callee_regs_load \base,    2

    ld_base          t6,       2 + CALLEE_SAVE_REGS, \base
    csrw             satp,     t6

    ld_base          t6,       3 + CALLEE_SAVE_REGS, \base
    csrw             sepc,     t6

    ld_base          t6,       4 + CALLEE_SAVE_REGS, \base
    csrw             sstatus,  t6

    ld_base          t6,       5 + CALLEE_SAVE_REGS, \base
    csrw             sie,      t6

    ld_base          t6,       6 + CALLEE_SAVE_REGS, \base
    csrw             sip,      t6

    ld_base          t6,       7 + CALLEE_SAVE_REGS, \base
    csrw             sscratch, t6
.endm

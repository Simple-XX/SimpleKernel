
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

#include "macro.S"

// clang-format off

/**
 * @brief 保存所有寄存器
 * @param  _base           要保存到的基地址
 */
.macro SaveAllRegs _base=sp
    // 在栈上留出保存寄存器的空间
    addi \_base, \_base, -kAllRegsSize

    SdBase  zero, 0,  \_base
    SdBase  ra,   1,  \_base
    
    // 这里需要保存旧寄存器的值
    // 使用 ra 做临时变量
    addi    ra, \_base, kAllRegsSize
    SdBase  ra,   2,  \_base
    // 恢复 ra
    LdBase  ra,   1,  \_base

    SdBase  gp,   3,  \_base
    SdBase  tp,   4,  \_base
    SdBase  t0,   5,  \_base
    SdBase  t1,   6,  \_base
    SdBase  t2,   7,  \_base
    SdBase  s0,   8,  \_base
    SdBase  s1,   9,  \_base
    SdBase  a0,   10, \_base
    SdBase  a1,   11, \_base
    SdBase  a2,   12, \_base
    SdBase  a3,   13, \_base
    SdBase  a4,   14, \_base
    SdBase  a5,   15, \_base
    SdBase  a6,   16, \_base
    SdBase  a7,   17, \_base
    SdBase  s2,   18, \_base
    SdBase  s3,   19, \_base
    SdBase  s4,   20, \_base
    SdBase  s5,   21, \_base
    SdBase  s6,   22, \_base
    SdBase  s7,   23, \_base
    SdBase  s8,   24, \_base
    SdBase  s9,   25, \_base
    SdBase  s10,  26, \_base
    SdBase  s11,  27, \_base
    SdBase  t3,   28, \_base
    SdBase  t4,   29, \_base
    SdBase  t5,   30, \_base
    SdBase  t6,   31, \_base

    FsdBase ft0,  32, \_base
    FsdBase ft1,  33, \_base
    FsdBase ft2,  34, \_base
    FsdBase ft3,  35, \_base
    FsdBase ft4,  36, \_base
    FsdBase ft5,  37, \_base
    FsdBase ft6,  38, \_base
    FsdBase ft7,  39, \_base
    FsdBase fs0,  40, \_base
    FsdBase fs1,  41, \_base
    FsdBase fa0,  42, \_base
    FsdBase fa1,  43, \_base
    FsdBase fa2,  44, \_base
    FsdBase fa3,  45, \_base
    FsdBase fa4,  46, \_base
    FsdBase fa5,  47, \_base
    FsdBase fa6,  48, \_base
    FsdBase fa7,  49, \_base
    FsdBase fs2,  50, \_base
    FsdBase fs3,  51, \_base
    FsdBase fs4,  52, \_base
    FsdBase fs5,  53, \_base
    FsdBase fs6,  54, \_base
    FsdBase fs7,  55, \_base
    FsdBase fs8,  56, \_base
    FsdBase fs9,  57, \_base
    FsdBase fs10, 58, \_base
    FsdBase fs11, 59, \_base
    FsdBase ft8,  60, \_base
    FsdBase ft9,  61, \_base
    FsdBase ft10, 62, \_base
    FsdBase ft11, 63, \_base

    // 使用 ra 暂存数据
    csrr     ra,   sepc
    SdBase   ra,   64, \_base
    csrr     ra,   stval
    SdBase   ra,   65, \_base
    csrr     ra,   scause
    SdBase   ra,   66, \_base
    csrr     ra,   sie
    SdBase   ra,   67, \_base
    csrr     ra,   sstatus
    SdBase   ra,   68, \_base
    csrr     ra,   satp
    SdBase   ra,   69, \_base
    csrr     ra,   sscratch
    SdBase   ra,   70, \_base
    // 恢复 ra
    LdBase   ra,   1,  \_base
.endm

/**
 * @brief 恢复所有寄存器
 * @param  _base           要恢复数据的基地址
 */
.macro LoadAllRegs _base=sp
    LdBase  zero, 0,  \_base
    LdBase  ra,   1,  \_base
    // sp 还保存有数据，暂不恢复
    // LdBase  sp,   2,  \_base
    LdBase  gp,   3,  \_base
    LdBase  tp,   4,  \_base
    LdBase  t0,   5,  \_base
    LdBase  t1,   6,  \_base
    LdBase  t2,   7,  \_base
    LdBase  s0,   8,  \_base
    LdBase  s1,   9,  \_base
    LdBase  a0,   10, \_base
    LdBase  a1,   11, \_base
    LdBase  a2,   12, \_base
    LdBase  a3,   13, \_base
    LdBase  a4,   14, \_base
    LdBase  a5,   15, \_base
    LdBase  a6,   16, \_base
    LdBase  a7,   17, \_base
    LdBase  s2,   18, \_base
    LdBase  s3,   19, \_base
    LdBase  s4,   20, \_base
    LdBase  s5,   21, \_base
    LdBase  s6,   22, \_base
    LdBase  s7,   23, \_base
    LdBase  s8,   24, \_base
    LdBase  s9,   25, \_base
    LdBase  s10,  26, \_base
    LdBase  s11,  27, \_base
    LdBase  t3,   28, \_base
    LdBase  t4,   29, \_base
    LdBase  t5,   30, \_base
    LdBase  t6,   31, \_base

    FldBase ft0,  32, \_base
    FldBase ft1,  33, \_base
    FldBase ft2,  34, \_base
    FldBase ft3,  35, \_base
    FldBase ft4,  36, \_base
    FldBase ft5,  37, \_base
    FldBase ft6,  38, \_base
    FldBase ft7,  39, \_base
    FldBase fs0,  40, \_base
    FldBase fs1,  41, \_base
    FldBase fa0,  42, \_base
    FldBase fa1,  43, \_base
    FldBase fa2,  44, \_base
    FldBase fa3,  45, \_base
    FldBase fa4,  46, \_base
    FldBase fa5,  47, \_base
    FldBase fa6,  48, \_base
    FldBase fa7,  49, \_base
    FldBase fs2,  50, \_base
    FldBase fs3,  51, \_base
    FldBase fs4,  52, \_base
    FldBase fs5,  53, \_base
    FldBase fs6,  54, \_base
    FldBase fs7,  55, \_base
    FldBase fs8,  56, \_base
    FldBase fs9,  57, \_base
    FldBase fs10, 58, \_base
    FldBase fs11, 59, \_base
    FldBase ft8,  60, \_base
    FldBase ft9,  61, \_base
    FldBase ft10, 62, \_base
    FldBase ft11, 63, \_base

    // 使用 ra 暂存数据
    LdBase   ra,   64, \_base
    csrw     sepc,     ra
    LdBase   ra,   65, \_base
    csrw     stval,    ra
    LdBase   ra,   66, \_base
    csrw     scause,   ra
    LdBase   ra,   67, \_base
    csrw     sie,  ra
    LdBase   ra,   68, \_base
    csrw     sstatus,  ra
    LdBase   ra,   69, \_base
    csrw     satp,     ra
    LdBase   ra,   70, \_base
    csrw     sscratch, ra
    // 恢复 ra
    LdBase   ra,   1,  \_base

    // 恢复 sp
    LdBase   sp,   2,  \_base

    // 释放栈上用于保存寄存器的空间
    addi \_base, \_base, kAllRegsSize
.endm

/**
 * @brief 保存调用者保存寄存器
 * @param  _base           要保存到的基地址
 * @param  _off:           基地址的偏移
 */
.macro SaveCallerRegs _base, _off
    SdBase  ra,   0  + \_off, \_base
    SdBase  t0,   1  + \_off, \_base
    SdBase  t2,   2  + \_off, \_base
    SdBase  a0,   3  + \_off, \_base
    SdBase  a1,   4  + \_off, \_base
    SdBase  a2,   5  + \_off, \_base
    SdBase  a3,   6  + \_off, \_base
    SdBase  a4,   7  + \_off, \_base
    SdBase  a5,   8  + \_off, \_base
    SdBase  a6,   9  + \_off, \_base
    SdBase  a7,   10 + \_off, \_base
    SdBase  t3,   11 + \_off, \_base
    SdBase  t4,   12 + \_off, \_base
    SdBase  t5,   13 + \_off, \_base
    SdBase  t6,   14 + \_off, \_base
    FsdBase ft0,  15 + \_off, \_base
    FsdBase ft1,  16 + \_off, \_base
    FsdBase ft2,  17 + \_off, \_base
    FsdBase ft3,  18 + \_off, \_base
    FsdBase ft4,  19 + \_off, \_base
    FsdBase ft5,  20 + \_off, \_base
    FsdBase ft6,  21 + \_off, \_base
    FsdBase ft7,  22 + \_off, \_base
    FsdBase fa0,  23 + \_off, \_base
    FsdBase fa1,  24 + \_off, \_base
    FsdBase fa2,  25 + \_off, \_base
    FsdBase fa3,  26 + \_off, \_base
    FsdBase fa4,  27 + \_off, \_base
    FsdBase fa5,  28 + \_off, \_base
    FsdBase fa6,  29 + \_off, \_base
    FsdBase fa7,  30 + \_off, \_base
    FsdBase ft8,  31 + \_off, \_base
    FsdBase ft9,  32 + \_off, \_base
    FsdBase ft10, 33 + \_off, \_base
    FsdBase ft11, 34 + \_off, \_base
.endm

/**
 * @brief 恢复调用者保存寄存器
 * @param  _base           要恢复的基地址
 * @param  _off:           基地址的偏移
 */
.macro LoadCallerRegs _base, _off
    LdBase  ra,   0  + \_off, \_base
    LdBase  t0,   1  + \_off, \_base
    LdBase  t2,   2  + \_off, \_base
    LdBase  a0,   3  + \_off, \_base
    LdBase  a1,   4  + \_off, \_base
    LdBase  a2,   5  + \_off, \_base
    LdBase  a3,   6  + \_off, \_base
    LdBase  a4,   7  + \_off, \_base
    LdBase  a5,   8  + \_off, \_base
    LdBase  a6,   9  + \_off, \_base
    LdBase  a7,   10 + \_off, \_base
    LdBase  t3,   11 + \_off, \_base
    LdBase  t4,   12 + \_off, \_base
    LdBase  t5,   13 + \_off, \_base
    LdBase  t6,   14 + \_off, \_base
    FldBase ft0,  15 + \_off, \_base
    FldBase ft1,  16 + \_off, \_base
    FldBase ft2,  17 + \_off, \_base
    FldBase ft3,  18 + \_off, \_base
    FldBase ft4,  19 + \_off, \_base
    FldBase ft5,  20 + \_off, \_base
    FldBase ft6,  21 + \_off, \_base
    FldBase ft7,  22 + \_off, \_base
    FldBase fa0,  23 + \_off, \_base
    FldBase fa1,  24 + \_off, \_base
    FldBase fa2,  25 + \_off, \_base
    FldBase fa3,  26 + \_off, \_base
    FldBase fa4,  27 + \_off, \_base
    FldBase fa5,  28 + \_off, \_base
    FldBase fa6,  29 + \_off, \_base
    FldBase fa7,  30 + \_off, \_base
    FldBase ft8,  31 + \_off, \_base
    FldBase ft9,  32 + \_off, \_base
    FldBase ft10, 33 + \_off, \_base
    FldBase ft11, 34 + \_off, \_base
.endm

/**
 * @brief 保存被调用者保存寄存器
 * @param  _base           要保存的基地址
 * @param  _off:           基地址的偏移
 */
.macro SaveCaleeRegs _base, _off
    SdBase  sp,   0  + \_off, \_base
    SdBase  s0,   1  + \_off, \_base
    SdBase  s1,   2  + \_off, \_base
    SdBase  s2,   3  + \_off, \_base
    SdBase  s3,   4  + \_off, \_base
    SdBase  s4,   5  + \_off, \_base
    SdBase  s5,   6  + \_off, \_base
    SdBase  s6,   7  + \_off, \_base
    SdBase  s7,   8  + \_off, \_base
    SdBase  s8,   9  + \_off, \_base
    SdBase  s9,   10 + \_off, \_base
    SdBase  s10,  11 + \_off, \_base
    SdBase  s11,  12 + \_off, \_base
    FsdBase fs0,  13 + \_off, \_base
    FsdBase fs1,  14 + \_off, \_base
    FsdBase fs2,  15 + \_off, \_base
    FsdBase fs3,  16 + \_off, \_base
    FsdBase fs4,  17 + \_off, \_base
    FsdBase fs5,  18 + \_off, \_base
    FsdBase fs6,  19 + \_off, \_base
    FsdBase fs7,  20 + \_off, \_base
    FsdBase fs8,  21 + \_off, \_base
    FsdBase fs9,  22 + \_off, \_base
    FsdBase fs10, 23 + \_off, \_base
    FsdBase fs11, 24 + \_off, \_base
.endm

/**
 * @brief 恢复被调用者保存寄存器
 * @param  _base           要恢复数据的基地址
 * @param  _off:           基地址的偏移
 */
.macro LoadCalleeRegs _base, _off
    LdBase  sp,   0  + \_off, \_base
    LdBase  s0,   1  + \_off, \_base
    LdBase  s1,   2  + \_off, \_base
    LdBase  s2,   3  + \_off, \_base
    LdBase  s3,   4  + \_off, \_base
    LdBase  s4,   5  + \_off, \_base
    LdBase  s5,   6  + \_off, \_base
    LdBase  s6,   7  + \_off, \_base
    LdBase  s7,   8  + \_off, \_base
    LdBase  s8,   9  + \_off, \_base
    LdBase  s9,   10 + \_off, \_base
    LdBase  s10,  11 + \_off, \_base
    LdBase  s11,  12 + \_off, \_base
    FldBase fs0,  13 + \_off, \_base
    FldBase fs1,  14 + \_off, \_base
    FldBase fs2,  15 + \_off, \_base
    FldBase fs3,  16 + \_off, \_base
    FldBase fs4,  17 + \_off, \_base
    FldBase fs5,  18 + \_off, \_base
    FldBase fs6,  19 + \_off, \_base
    FldBase fs7,  20 + \_off, \_base
    FldBase fs8,  21 + \_off, \_base
    FldBase fs9,  22 + \_off, \_base
    FldBase fs10, 23 + \_off, \_base
    FldBase fs11, 24 + \_off, \_base
.endm

/**
 * @brief 保存 context_t
 * @param  _base           要保存到的基地址
 */
.macro SaveContext _base
    // @todo 临时设置为 tp
    SdBase          tp,       0,                    \_base
    SdBase          ra,       1,                    \_base
    SaveCaleeRegs \_base,   2

    csrr             t6,       satp
    SdBase          t6,       2 + kCalleeSaveRegsCount, \_base

    csrr             t6,       sepc
    SdBase          t6,       3 + kCalleeSaveRegsCount, \_base

    csrr             t6,       sstatus
    SdBase          t6,       4 + kCalleeSaveRegsCount, \_base

    csrr             t6,       sie
    SdBase          t6,       5 + kCalleeSaveRegsCount, \_base

    csrr             t6,       sip
    SdBase          t6,       6 + kCalleeSaveRegsCount, \_base

    csrr             t6,       sscratch
    SdBase          t6,       7 + kCalleeSaveRegsCount, \_base
.endm

/**
 * @brief 恢复 context_t
 * @param  _base           要恢复数据的基地址
 */
.macro LoadContext _base
    // @todo 临时设置为 tp
    LdBase          tp,       0,                    \_base
    LdBase          ra,       1,                    \_base
    LoadCalleeRegs \_base,   2

    LdBase          t6,       2 + kCalleeSaveRegsCount, \_base
    csrw             satp,     t6

    LdBase          t6,       3 + kCalleeSaveRegsCount, \_base
    csrw             sepc,     t6

    LdBase          t6,       4 + kCalleeSaveRegsCount, \_base
    csrw             sstatus,  t6

    LdBase          t6,       5 + kCalleeSaveRegsCount, \_base
    csrw             sie,      t6

    LdBase          t6,       6 + kCalleeSaveRegsCount, \_base
    csrw             sip,      t6

    LdBase          t6,       7 + kCalleeSaveRegsCount, \_base
    csrw             sscratch, t6
.endm


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
 * @brief 保存调用者保存寄存器
 * @param  _base           要保存到的基地址
 * @param  _off            基地址的偏移
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
 * @param  _off            基地址的偏移
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
 * @param  _off            基地址的偏移
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
 * @param  _off            基地址的偏移
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

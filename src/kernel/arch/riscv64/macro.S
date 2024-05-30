
/**
 * @file macro.S
 * @brief macro S 汇编宏
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-04-24
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-04-24<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

// clang-format off

// 寄存器长度，8 字节
.equ kRegsBytes, 8
// 所有寄存器数量
.equ kAllRegsCount, 71
// 保存所有寄存器需要的大小
.equ kAllRegSize, (kAllRegsCount * kRegsBytes)

// 将寄存器 a 保存在 c 偏移 b 的位置
.macro SdBase a, b, c
sd \a, ((\b) * kRegsBytes)(\c)
.endm

// 从 c 的偏移 b 处获取数据并赋值给寄存器 a
.macro LdBase a, b, c
ld \a, ((\b) * kRegsBytes)(\c)
.endm

// 将 float 寄存器 a 保存在 c 偏移 b 的位置
.macro FsdBase a, b, c
fsd \a, ((\b) * kRegsBytes)(\c)
.endm

// 从 c 的偏移 b 处获取数据并赋值给 float 寄存器 a
.macro FldBase a, b, c
fld \a, ((\b) * kRegsBytes)(\c)
.endm

// clang-format on

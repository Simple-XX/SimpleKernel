
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
.equ REG_BYTES, 8
// 所有寄存器数量
.equ ALL_REGS,  71
// 保存所有寄存器需要的大小
.equ ALL_SIZE,  (ALL_REGS * REG_BYTES)

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

// clang-format on

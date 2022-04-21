
/**
 * @file context_switch.S
 * @brief 上下文切换定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-02<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

// 寄存器长度，8 字节
.equ REG_BYTES, 8
// 保存所有寄存器
.equ CALLEE_SAVE_REGS, 25
.equ SAVE_REGS, 32
// 需要的大小
.equ CONTEXT_SIZE, (SAVE_REGS * REG_BYTES)
.equ CALLEE_SIZE, (CALLEE_SAVE_REGS * REG_BYTES)

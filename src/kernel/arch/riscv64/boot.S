
/**
 * @file boot.S
 * @brief boot S
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include "macro.S"

// clang-format off

.section .text.boot
.global _boot
.type _boot, @function
.extern _start
_boot:
    // 设置栈地址
    la sp, stack_top

    // 保存 sbi 传递的参数
    // 开辟栈空间
    addi sp, sp, -8*2
    // a0 为启动核 id
    sd_base a0, 0, sp
    // a1 为 dtb 地址
    sd_base a1, 0, sp

    // 跳转到 C 代码执行
    call _start
loop:
    j loop

// 声明所属段
.section .bss.boot
// 16 字节对齐
.align 16
.global stack_top
stack_top:
    // 跳过 16KB
    .space 4096 * 4

// clang-format on

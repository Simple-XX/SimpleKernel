
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
#if USE_NO_RELAX == 0
.extern __global_pointer$
#endif
_boot:

#if USE_NO_RELAX == 0
    // 初始化 gp 寄存器
    // 如果不初始化，则需要在编译时添加 -mno-relax 选项
.option push
.option norelax
    la gp, __global_pointer$
.option pop
#endif

    // 设置栈地址
    la sp, stack_top

    // 保存 sbi 传递的参数
    // 开辟栈空间
    addi sp, sp, -8*2
    // a0 为启动核 id
    SdBase a0, 0, sp
    // a1 为 dtb 地址
    SdBase a1, 1, sp

    // 跳转到 C 代码执行
    call _start

    wfi

// 声明所属段
.section .bss.boot
// 16 字节对齐
.align 16
.global stack_top
stack_top:
    // 跳过 16KB
    .space 4096 * 4

// clang-format on

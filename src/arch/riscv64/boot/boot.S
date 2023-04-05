
/**
 * @file boot.S
 * @brief 启动代码，进行一些设置后跳转到 kernel_main
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

.section .text.boot
.global _start
.type _start, @function
.extern kernel_main
.extern cpp_init
_start:
    // 设置栈地址
    la sp, stack_top
    // 初始化 C++
    call cpp_init
    // 跳转到 CPP 代码执行
    call kernel_main
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


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

.section .init
.globl _start
.type _start, @function
.extern kernel_main
.extern cpp_init
.extern boot_info_addr
.extern dtb_init_hart
_start:
    // 保存 sbi 传递的参数
    // 将 a0 的值传递给 dtb_init_hart
    sw a0, dtb_init_hart, t0
    // 将 a1 的值传递给 boot_info_addr
    sw a1, boot_info_addr, t0
    // 为每个 core 计算栈地址
    add t0, a0, 1
    slli t0, t0, 12
    // 设置栈
    la sp, stack_top
    add sp, sp, t0
    // 跳转到 kernel_main
    call kernel_main
loop:
    j loop

// 声明所属段
.section .bss
// 16 字节对齐
.align 16
.global stack_top
stack_top:
    // 跳过 8KB
    .space 4096 * 2

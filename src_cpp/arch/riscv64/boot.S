/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "macro.S"

.section .text.boot
.global _boot
.type _boot, @function
.extern _start
#if USE_NO_RELAX == 0
.extern __global_pointer$
#endif
_boot:
    // Check if dtb address (a1) is zero
    beqz a1, 2f
#if USE_NO_RELAX == 0
/// @see riscv-abi.pdf#9.1.4
/// https://github.com/riscv-non-isa/riscv-elf-psabi-doc/releases/tag/v1.0
    // 初始化 gp 寄存器
    // 如果不初始化，则需要在编译时添加 -mno-relax 选项
.option push
.option norelax
1:  auipc gp, %pcrel_hi(__global_pointer$)
    addi  gp, gp, %pcrel_lo(1b)
.option pop
#endif

2:  // 按照每个 core 设置栈地址
    add t0, a0, 1
    // 根据 SIMPLEKERNEL_DEFAULT_STACK_SIZE 计算每核栈偏移
    li t1, SIMPLEKERNEL_DEFAULT_STACK_SIZE
    mul t0, t0, t1
    la sp, stack_top
    add sp, sp, t0

    // 将 a0 的值写入 tp
    mv tp, a0
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
    .space SIMPLEKERNEL_DEFAULT_STACK_SIZE * SIMPLEKERNEL_MAX_CORE_COUNT

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

.section .text.boot
.global _boot
.type _boot, @function
.extern _start
_boot:
    // 获取启动核 ID
    mrs x10, mpidr_el1
    and x10, x10, #0xFF

    // 按照每个 core 设置栈地址
    add x10, x10, #1
    // 根据 SIMPLEKERNEL_DEFAULT_STACK_SIZE 计算每核栈偏移
    ldr x12, =SIMPLEKERNEL_DEFAULT_STACK_SIZE
    mul x10, x10, x12
    ldr x11, =stack_top
    add x11, x11, x10
    mov sp, x11

    // 保存传递的参数（假设传递的参数在 x0 和 x1 中）
    // 开辟栈空间
    stp x0, x1, [sp, #-16]!

    bl _start
    b .

// 声明所属段
.section .bss.boot
// 16 字节对齐
.align 16
.global stack_top
stack_top:
    .space SIMPLEKERNEL_DEFAULT_STACK_SIZE * SIMPLEKERNEL_MAX_CORE_COUNT

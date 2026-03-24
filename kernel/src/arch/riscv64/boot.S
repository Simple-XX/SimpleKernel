#include "macro.S"

.section .text.boot
.global _boot
.type _boot, @function
.extern _start

_boot:
    addi t0, a0, 1
    li t1, SIMPLEKERNEL_DEFAULT_STACK_SIZE
    mul t0, t0, t1
    la sp, stack_top
    add sp, sp, t0

    mv tp, a0
    addi sp, sp, -16
    SdBase a0, 0, sp
    SdBase a1, 1, sp

    call _start

1:
    wfi
    j 1b

.section .bss.boot
.balign 16
.global stack_top
stack_top:
    .space SIMPLEKERNEL_DEFAULT_STACK_SIZE * SIMPLEKERNEL_MAX_CORE_COUNT

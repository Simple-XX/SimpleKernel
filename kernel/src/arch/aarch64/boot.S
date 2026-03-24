.section .text.boot
.global _boot
.type _boot, @function
.extern _start

_boot:
    mrs x10, mpidr_el1
    and x10, x10, #0xFF

    add x10, x10, #1
    ldr x12, =SIMPLEKERNEL_DEFAULT_STACK_SIZE
    mul x10, x10, x12
    ldr x11, =stack_top
    add x11, x11, x10
    mov sp, x11

    stp x0, x1, [sp, #-16]!

    bl _start

1:
    wfi
    b 1b

.section .bss.boot
.balign 16
.global stack_top
stack_top:
    .space SIMPLEKERNEL_DEFAULT_STACK_SIZE * SIMPLEKERNEL_MAX_CORE_COUNT

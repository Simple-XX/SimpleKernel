#define ALIGN_16(x) (((x) + 15) & ~15)

.equ kRegsBytes, 8
.equ kTrapContextSize, 320
.equ kCalleeSavedContextSize, 112

.macro SaveTrapContext
    sub sp, sp, #kTrapContextSize
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #240]
    mrs x9, elr_el1
    mrs x10, spsr_el1
    stp x9, x10, [sp, #256]
    mrs x9, esr_el1
    str x9, [sp, #272]
    mrs x9, sp_el0
    mrs x10, tpidr_el0
    stp x9, x10, [sp, #280]
    mrs x9, ttbr0_el1
    str x9, [sp, #296]
    add x9, sp, #kTrapContextSize
    mrs x10, tpidr_el1
    stp x9, x10, [sp, #304]
.endm

.macro RestoreTrapContext
    ldp x9, x10, [sp, #256]
    msr elr_el1, x9
    msr spsr_el1, x10
    ldp x9, x10, [sp, #280]
    ldr x11, [sp, #296]
    msr sp_el0, x9
    msr tpidr_el0, x10
    msr ttbr0_el1, x11
    ldp x9, x10, [sp, #304]
    msr tpidr_el1, x10
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #240]
    add sp, sp, #kTrapContextSize
.endm

.macro SaveCalleeSavedContext base_reg
    stp x19, x20, [\base_reg, #0]
    stp x21, x22, [\base_reg, #16]
    stp x23, x24, [\base_reg, #32]
    stp x25, x26, [\base_reg, #48]
    stp x27, x28, [\base_reg, #64]
    stp x29, x30, [\base_reg, #80]
    mov x9, sp
    mov x10, x30
    stp x9, x10, [\base_reg, #96]
.endm

.macro RestoreCalleeSavedContext base_reg
    ldp x9, x10, [\base_reg, #96]
    mov sp, x9
    ldp x19, x20, [\base_reg, #0]
    ldp x21, x22, [\base_reg, #16]
    ldp x23, x24, [\base_reg, #32]
    ldp x25, x26, [\base_reg, #48]
    ldp x27, x28, [\base_reg, #64]
    ldp x29, x30, [\base_reg, #80]
    br x10
.endm

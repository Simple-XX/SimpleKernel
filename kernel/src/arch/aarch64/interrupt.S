#include "macro.S"

.section .text

.global vector_table
.balign 0x800
vector_table:
.balign 0x80
    b sync_current_el_sp0
.balign 0x80
    b irq_current_el_sp0
.balign 0x80
    b fiq_current_el_sp0
.balign 0x80
    b error_current_el_sp0

.balign 0x80
    b sync_current_el_spx
.balign 0x80
    b irq_current_el_spx
.balign 0x80
    b fiq_current_el_spx
.balign 0x80
    b error_current_el_spx

.balign 0x80
    b sync_lower_el_aarch64
.balign 0x80
    b irq_lower_el_aarch64
.balign 0x80
    b fiq_lower_el_aarch64
.balign 0x80
    b error_lower_el_aarch64

.balign 0x80
    b sync_lower_el_aarch32
.balign 0x80
    b irq_lower_el_aarch32
.balign 0x80
    b fiq_lower_el_aarch32
.balign 0x80
    b error_lower_el_aarch32

sync_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl sync_current_el_sp0_handler
    RestoreTrapContext
    eret

irq_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl irq_current_el_sp0_handler
    RestoreTrapContext
    eret

fiq_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl fiq_current_el_sp0_handler
    RestoreTrapContext
    eret

error_current_el_sp0:
    SaveTrapContext
    mov x0, sp
    bl error_current_el_sp0_handler
    RestoreTrapContext
    eret

sync_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl sync_current_el_spx_handler
    RestoreTrapContext
    eret

irq_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl irq_current_el_spx_handler
    RestoreTrapContext
    eret

fiq_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl fiq_current_el_spx_handler
    RestoreTrapContext
    eret

error_current_el_spx:
    SaveTrapContext
    mov x0, sp
    bl error_current_el_spx_handler
    RestoreTrapContext
    eret

sync_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl sync_lower_el_aarch64_handler
    RestoreTrapContext
    eret

irq_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl irq_lower_el_aarch64_handler
    RestoreTrapContext
    eret

fiq_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl fiq_lower_el_aarch64_handler
    RestoreTrapContext
    eret

error_lower_el_aarch64:
    SaveTrapContext
    mov x0, sp
    bl error_lower_el_aarch64_handler
    RestoreTrapContext
    eret

sync_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl sync_lower_el_aarch32_handler
    RestoreTrapContext
    eret

irq_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl irq_lower_el_aarch32_handler
    RestoreTrapContext
    eret

fiq_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl fiq_lower_el_aarch32_handler
    RestoreTrapContext
    eret

error_lower_el_aarch32:
    SaveTrapContext
    mov x0, sp
    bl error_lower_el_aarch32_handler
    RestoreTrapContext
    eret

.global trap_return
.type trap_return, @function
trap_return:
    mov sp, x0
    ldr x9, [sp, #264]
    and x9, x9, #0xF
    cbnz x9, .Lret_to_kernel

.Lret_to_user:
    ldr x9, [sp, #296]
    msr ttbr0_el1, x9
    isb
    RestoreTrapContext
    eret

.Lret_to_kernel:
    RestoreTrapContext
    eret

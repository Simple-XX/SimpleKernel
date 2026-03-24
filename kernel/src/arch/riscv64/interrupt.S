#include "macro.S"

.section .text

.global trap_return
.type trap_return, @function

.global trap_entry
.type trap_entry, @function

.extern HandleTrap

.align 4

trap_entry:
    csrrw sp, sscratch, sp
    bnez sp, .Ltrap_from_user
    csrrw sp, sscratch, sp
    j .Lsave_context

.Ltrap_from_user:

.Lsave_context:
    SaveTrapContext
    csrr t0, sscratch
    beqz t0, .Lcall_handler
    sd t0, 8(sp)
    csrw sscratch, x0

.Lcall_handler:
    mv a0, sp
    call HandleTrap

trap_return:
    mv sp, a0
    ld t0, 31*8(sp)
    andi t0, t0, 0x100
    bnez t0, .Lret_to_kernel

.Lret_to_user:
    ld t0, 8(sp)
    csrw sscratch, t0
    RestoreTrapContext
    csrrw sp, sscratch, sp
    sret

.Lret_to_kernel:
    RestoreTrapContext
    sret

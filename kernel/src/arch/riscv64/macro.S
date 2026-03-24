#define ALIGN_16(x) (((x) + 15) & ~15)

.equ kRegsBytes, 8
.equ kTrapContextRegsCount, 36
.equ kTrapContextSize, ALIGN_16(kTrapContextRegsCount * kRegsBytes)
.equ kCalleeSavedContextRegsCount, 14
.equ kCalleeSavedContextSize, ALIGN_16(kCalleeSavedContextRegsCount * kRegsBytes)

.macro SdBase a, b, c
    sd \a, ((\b) * kRegsBytes)(\c)
.endm

.macro LdBase a, b, c
    ld \a, ((\b) * kRegsBytes)(\c)
.endm

.macro SaveTrapContext
    addi sp, sp, -kTrapContextSize
    SdBase ra, 0, sp
    SdBase gp, 2, sp
    SdBase tp, 3, sp
    SdBase t0, 4, sp
    SdBase t1, 5, sp
    SdBase t2, 6, sp
    SdBase s0, 7, sp
    SdBase s1, 8, sp
    SdBase a0, 9, sp
    SdBase a1, 10, sp
    SdBase a2, 11, sp
    SdBase a3, 12, sp
    SdBase a4, 13, sp
    SdBase a5, 14, sp
    SdBase a6, 15, sp
    SdBase a7, 16, sp
    SdBase s2, 17, sp
    SdBase s3, 18, sp
    SdBase s4, 19, sp
    SdBase s5, 20, sp
    SdBase s6, 21, sp
    SdBase s7, 22, sp
    SdBase s8, 23, sp
    SdBase s9, 24, sp
    SdBase s10, 25, sp
    SdBase s11, 26, sp
    SdBase t3, 27, sp
    SdBase t4, 28, sp
    SdBase t5, 29, sp
    SdBase t6, 30, sp
    addi t0, sp, kTrapContextSize
    SdBase t0, 1, sp
    csrr t0, sstatus
    SdBase t0, 31, sp
    csrr t0, sepc
    SdBase t0, 32, sp
    csrr t0, stval
    SdBase t0, 33, sp
    csrr t0, scause
    SdBase t0, 34, sp
.endm

.macro RestoreTrapContext
    LdBase t0, 31, sp
    csrw sstatus, t0
    LdBase t0, 32, sp
    csrw sepc, t0
    LdBase t0, 33, sp
    csrw stval, t0
    LdBase t0, 34, sp
    csrw scause, t0
    LdBase ra, 0, sp
    LdBase gp, 2, sp
    LdBase tp, 3, sp
    LdBase t1, 5, sp
    LdBase t2, 6, sp
    LdBase s0, 7, sp
    LdBase s1, 8, sp
    LdBase a0, 9, sp
    LdBase a1, 10, sp
    LdBase a2, 11, sp
    LdBase a3, 12, sp
    LdBase a4, 13, sp
    LdBase a5, 14, sp
    LdBase a6, 15, sp
    LdBase a7, 16, sp
    LdBase s2, 17, sp
    LdBase s3, 18, sp
    LdBase s4, 19, sp
    LdBase s5, 20, sp
    LdBase s6, 21, sp
    LdBase s7, 22, sp
    LdBase s8, 23, sp
    LdBase s9, 24, sp
    LdBase s10, 25, sp
    LdBase s11, 26, sp
    LdBase t3, 27, sp
    LdBase t4, 28, sp
    LdBase t5, 29, sp
    LdBase t6, 30, sp
    LdBase t0, 4, sp
    addi sp, sp, kTrapContextSize
.endm

.macro SaveCalleeSavedContext base_reg
    SdBase ra, 0, \base_reg
    SdBase sp, 1, \base_reg
    SdBase s0, 2, \base_reg
    SdBase s1, 3, \base_reg
    SdBase s2, 4, \base_reg
    SdBase s3, 5, \base_reg
    SdBase s4, 6, \base_reg
    SdBase s5, 7, \base_reg
    SdBase s6, 8, \base_reg
    SdBase s7, 9, \base_reg
    SdBase s8, 10, \base_reg
    SdBase s9, 11, \base_reg
    SdBase s10, 12, \base_reg
    SdBase s11, 13, \base_reg
.endm

.macro RestoreCalleeSavedContext base_reg
    LdBase ra, 0, \base_reg
    LdBase sp, 1, \base_reg
    LdBase s0, 2, \base_reg
    LdBase s1, 3, \base_reg
    LdBase s2, 4, \base_reg
    LdBase s3, 5, \base_reg
    LdBase s4, 6, \base_reg
    LdBase s5, 7, \base_reg
    LdBase s6, 8, \base_reg
    LdBase s7, 9, \base_reg
    LdBase s8, 10, \base_reg
    LdBase s9, 11, \base_reg
    LdBase s10, 12, \base_reg
    LdBase s11, 13, \base_reg
.endm

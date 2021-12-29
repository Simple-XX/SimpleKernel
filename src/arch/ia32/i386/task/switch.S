
// This file is a part of Simple-XX/SimpleKernel 
// (https://github.com/Simple-XX/SimpleKernel).
//
// switch.S for Simple-XX/SimpleKernel.

.code32

.global switch_context

.macro popaq
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
.endm

switch_context:
    mov %rdi, %rsp
    mov %rsi, %rax
    popaq
    mov %rax, %cr3
    pop %rax
    iretq

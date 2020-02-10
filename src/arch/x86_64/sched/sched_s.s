
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# sched.s for MRNIU/SimpleKernel.

.global switch_to
switch_to:
    mov 4(%esp), %eax

    mov %esp, 0(%eax)
    mov %ebp, 4(%eax)
    mov %ebx, 8(%eax)
    mov %esi, 12(%eax)
    mov %edi, 16(%eax)
    pushf
    pop %ecx
    mov %ecx, 20(%eax)

    mov 8(%esp), %eax

    mov 0(%eax), %esp
    mov 4(%eax), %ebp
    mov 8(%eax), %ebx
    mov 12(%eax), %esi
    mov 16(%eax), %edi
    mov 20(%eax), %eax
    push %eax
    popf

    ret

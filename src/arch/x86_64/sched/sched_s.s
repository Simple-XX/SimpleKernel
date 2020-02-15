
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# sched.s for MRNIU/SimpleKernel.

# void switch_to(task_context_t * curr, task_context_t * next);
.global switch_to
switch_to:
    # 保存 prev
    mov 4(%esp), %eax
    # 保存 eip
    mov 4(%esp), %edx
    mov %edx, 0(%eax)
    # 保存 esp
    mov %esp, 4(%eax)
    # 保存 ebp
    mov %ebp, 8(%eax)
    # 保存 ebx
    mov %ebx, 12(%eax)
    # 保存 esi
    mov %esi, 16(%eax)
    # 保存 edi
    mov %edi, 20(%eax)
    # 保存 eflags
    pushf
    pop 24(%eax)

    # 恢复 curr
    mov 8(%esp), %eax
    # 填充 eip
    mov 0(%eax), %edx
    mov %edx, 8(%esp)
    # 填充 esp
    mov 4(%eax), %esp
    # 填充 ebp
    mov 8(%eax), %ebp
    # 填充 ebx
    mov 12(%eax), %ebx
    # 填充 esi
    mov 16(%eax), %esi
    # 填充 edi
    mov 20(%eax), %edi
    # 填充 eflags
    push 24(%eax)
    popf
    ret

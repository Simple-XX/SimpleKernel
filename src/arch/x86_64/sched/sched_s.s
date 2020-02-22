
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# sched.s for MRNIU/SimpleKernel.

# void switch_to(task_context_t * curr, task_context_t * next);
.global switch_to
.extern print_stack
switch_to:
    // push $10
    // call print_stack
    # 保存 prev
    mov -4(%esp), %eax
    # 保存 eip
    // mov -4(%esp), %edx
    // mov %edx, 0(%eax)
    # 保存 esp
    mov %esp, 4(%eax)
    # 保存 ebp
    mov %ebp, 8(%eax)
    # 保存 ebx
    mov %ebx, 12(%eax)
    # 保存 ecx
    mov %ecx, 16(%eax)
    # 保存 edx
    mov %edx, 20(%eax)
    # 保存 esi
    mov %esi, 24(%eax)
    # 保存 edi
    mov %edi, 28(%eax)
    // push $5
    // call print_stack
    # 恢复 curr
    mov -8(%esp), %eax
    // push $5
    // call print_stack
    # 填充 eip
    // mov 0(%eax), %edx
    // mov %edx, -8(%esp)
    # 填充 esp
    mov 4(%eax), %esp
    # 填充 ebp
    mov 8(%eax), %ebp
    # 填充 ebx
    mov 12(%eax), %ebx
    # 填充 ebx
    mov 16(%eax), %ecx
    # 填充 ebx
    mov 20(%eax), %edx
    # 填充 esi
    mov 24(%eax), %esi
    # 填充 edi
    mov 28(%eax), %edi
    // push $10
    // call print_stack
    // hlt
    ret

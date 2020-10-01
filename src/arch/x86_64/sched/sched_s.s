
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# sched.s for SimpleXX/SimpleKernel.

# void switch_to(task_pcb_t * curr, task_pcb_t * next);
// .global switch_to
// .extern print_stack
// .extern print_curr
// .extern print_next
// switch_to:
//     // push $1
//     // call print_stack
//     # 获取 curr 指针
//     mov -4(%esp), %eax
//     // 获取 curr->context 指针
//     mov 28(%eax), %eax
//     push %eax
//     call print_curr
//     # 保存 eip
//     // push %edx
//     // mov -4(%esp), %edx
//     // mov %edx, 0(%eax)
//     // pop %edx
//     # 保存 esp
//     mov %esp, 4(%eax)
//     # 保存 ebp
//     mov %ebp, 8(%eax)
//     # 保存 ebx
//     mov %ebx, 12(%eax)
//     # 保存 ecx
//     mov %ecx, 16(%eax)
//     # 保存 edx
//     mov %edx, 20(%eax)
//     # 保存 esi
//     mov %esi, 24(%eax)
//     # 保存 edi
//     mov %edi, 28(%eax)
//     # 恢复 curr
//     mov -8(%esp), %eax
//     mov 28(%eax), %eax
//     # 填充 eip
//     // push %edx
//     // mov 0(%eax), %edx
//     // mov %edx, -8(%esp)
//     // pop %edx
//     # 填充 esp
//     mov 4(%eax), %esp
//     // hlt
//     # 填充 ebp
//     mov 8(%eax), %ebp
//     # 填充 ebx
//     mov 12(%eax), %ebx
//     # 填充 ebx
//     mov 16(%eax), %ecx
//     # 填充 ebx
//     mov 20(%eax), %edx
//     # 填充 esi
//     mov 24(%eax), %esi
//     # 填充 edi
//     mov 28(%eax), %edi
//     ret

// switch_to:
//     // 在 eax，edx 寄存器中分别保存 curr 与 next 的值
//     mov curr, %eax
//     mov next, %edx
//     // 将 eflags 和 ebp 保存在 curr 的内核栈中
//     pushf
//     push %ebp
//     // 将 esp 保存到 prev->thread.esp，使该字段指向 prev 内核栈的栈顶
//     mov %esp, 484(%eax)
//     // 将 next->thread.esp 装入 esp。此时内核在 next 的内核栈上进行操作。
//     // 这条指令实际上完成了从 curr 到 next 的切换
//     mov 484(%edx), %esp
//     // 设置 eip，存入 curr->thread.eip，当 curr 重新恢复执行时，执行 $1f 指令
//     mov $1f, 480(%eax)
//     // 将 next->thread.eip 压入 next 的内核栈
//     push 480(%edx)
//     // 跳到 __switch_to()
//     jmp __switch_to
//     // 这里 curr 进程重新获得 CPU，保存 eflags 与 ebp
//     // 注意：当执行到这里，curr 作为 switch_to 的第二个参数，因此 esp 这时指向 curr 的内核栈
//     1:
//     pop %ebp
//     popf
//     // 拷贝 eax 到 last 中
//     mov %eax, last

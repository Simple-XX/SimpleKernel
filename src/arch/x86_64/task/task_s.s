
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# task_s.s for MRNIU/SimpleKernel.

.global kthread_entry
.extern do_exit
kthread_entry:
    mov %edx, %eax
    # 参数压栈
    push %edx
    # 执行函数
    call *%ebx
    # 保存返回值
    push %eax
    call do_exit

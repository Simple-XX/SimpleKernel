
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# task.s for MRNIU/SimpleKernel.

.global kthread_entry
.extern do_exit
kthread_entry:
    # 参数压栈
    push %edx
    # 要执行的函数地址压栈
    call *(%ebx)
    # 保存返回值
    push %eax
    call do_exit

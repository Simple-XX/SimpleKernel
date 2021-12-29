
// This file is a part of Simple-XX/SimpleKernel 
// (https://github.com/Simple-XX/SimpleKernel).
//
// task_s.S for Simple-XX/SimpleKernel.

.code32

.global kthread_entry
.extern do_exit
kthread_entry:
    mov %edx, %eax
    // 参数压栈
    push %edx
    // 执行函数
    call *%ebx
    // 保存返回值
    push %eax
    call do_exit

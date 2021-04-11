
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot64.s for Simple-XX/SimpleKernel.

.code64
.section .text
.global init
.extern kernel_main
init:
    jmp multiboot_entry

multiboot_entry:
    cli
	# 设置栈地址
    mov $STACK_TOP, %esp
    # 栈地址按照 16 字节对齐
    and $0xFFFFFFF0, %esp
    # 帧指针修改为 0
    mov $0, %ebp          
    push $0
    popf
	# multiboot2_info 结构体指针
    push %rbx
	# 魔数
	push %rax
    call kernel_main

1:
    hlt
    jmp 1b
    ret

.section .bss
STACK:
    .skip 16384
STACK_TOP:

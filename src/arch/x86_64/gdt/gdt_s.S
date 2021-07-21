
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# gdt_s.S for Simple-XX/SimpleKernel.

.code64

.section .text
.global gdt_load
gdt_load:
    cli
    # 加载到 GDTR，修改 boot.S 中的设置
    lgdt (%rdi)

# 加载数据段描述符
    mov $0x10, %rax
    # 更新所有可以更新的段寄存器
    mov %rax, %ds
    mov %rax, %es
    mov %rax, %fs
    mov %rax, %gs
    mov %rax, %ss
    call flush 

flush:
    ret

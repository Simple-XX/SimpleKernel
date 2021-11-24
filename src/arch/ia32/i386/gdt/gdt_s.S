
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// gdt_s.S for Simple-XX/SimpleKernel.

.code32

.section .text
.global gdt_load
gdt_load:
    cli
    // 参数存入 eax 寄存器
    mov 4(%esp), %eax
    // 加载到 GDTR [修改原先GRUB设置]
    lgdt (%eax)

// 加载数据段描述符
    mov $0x10, %ax
    // 更新所有可以更新的段寄存器
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    // 远跳转，0x08是代码段描述符
    jmp $0x08, $flush 
    // 远跳目的是清空流水线并串行化处理器

flush:
    ret

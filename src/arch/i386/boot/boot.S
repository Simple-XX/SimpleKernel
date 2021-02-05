
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot.s for Simple-XX/SimpleKernel.

#define ASM_FILE
#include "multiboot2.h"

.code32

.section .multiboot_header
# multiboot2 文件头

.SET HEADER_LENGTH, multiboot_header_end - multiboot_header
.SET CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + HEADER_LENGTH)
.align MULTIBOOT_HEADER_ALIGN
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC
    .long MULTIBOOT_ARCHITECTURE_I386
    .long HEADER_LENGTH
    .long CHECKSUM
    # 添加其它内容在此，详细信息见 Multiboot2 Specification version 2.0.pdf
	.short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long 8
multiboot_header_end:

.section .text
.global _start
.extern kernel_main
.type _start, @function
_start:
    jmp multiboot_entry
    ret

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
    push %ebx
	# 魔数
	push %eax
    call kernel_main

1:
    hlt
    jmp 1b
    ret

.section .bss
STACK:
    .skip 16384
STACK_TOP:

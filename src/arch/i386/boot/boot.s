
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
# boot.s for MRNIU/SimpleKernel.

# 设置栈大小
.set STACK_SIZE, 0x4000
# multiboot2 魔数字
.set MULTIBOOT2_HEADER_MAGIC, 0xE85250D6
# i386 架构标识
.set MULTIBOOT_ARCHITECTURE_I386, 0
# multiboot2 头结束标志
.set MULTIBOOT_HEADER_TAG_END, 0

.set MULTIBOOT_TAG_TYPE_ELF_SECTIONS, 9


.text
.align 8
.global start
.global multiboot2_info
.global multiboot2_magic
.extern kernel_main

# multiboot2 文件头
.align 8
multiboot_header:
  .long MULTIBOOT2_HEADER_MAGIC
  .long MULTIBOOT_ARCHITECTURE_I386
  .long multiboot_header_end - multiboot_header
  .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + multiboot_header_end - multiboot_header)

# 添加其它内容在此，见 Multiboot2 Specification version 2.0.pdf
/*
+-------------------+
u32     | type = 9          |
u32     | size              |
u16     | num               |
u16     | entsize           |
u16     | shndx             |
u16     | reserved          |
varies  | section headers   |
		 +-------------------+
.align 8
elf_symbols_tag:
	.long MULTIBOOT_TAG_TYPE_ELF_SECTIONS
	.long elf_symbols_tag_end - elf_symbols_tag
	.short
	.short
	.short
	.short

.align 8
elf_symbols_tag_end:
*/



.align 8
	.short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long 8
.align 8
multiboot_header_end:

#
.align 8
start:
.align 8
multiboot_entry:
	# 设置栈地址
  mov $(stack + STACK_SIZE), %esp
  push $0
  popf
	# multiboot2_info 结构体指针
  push %ebx
	# 魔数
	push %eax
  call kernel_main
.comm   stack, STACK_SIZE

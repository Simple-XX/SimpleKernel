
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
# gdt.h for MRNIU/SimpleKernel.

.global idt_load

idt_load:
  mov 4(%esp), %edx # 参数保存在 eax
	lidt (%edx)
	sti 				# turn on interrupts
	ret

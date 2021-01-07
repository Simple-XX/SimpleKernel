
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot32.s for Simple-XX/SimpleKernel.

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

.global _start32
.type _start32, @function
.extern init
_start32:
    jmp init
    ret

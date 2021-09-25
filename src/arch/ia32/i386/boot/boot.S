
// This file is a part of Simple-XX/SimpleKernel 
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot.S for Simple-XX/SimpleKernel.
// 启动代码，进行一些设置后跳转到 kernel_main

// 以下是来自 multiboot2 规范的定义
//  How many bytes from the start of the file we search for the header.
#define MULTIBOOT_SEARCH 32768
#define MULTIBOOT_HEADER_ALIGN 8

//  The magic field should contain this.
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

//  This should be in %eax.
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

//  Alignment of multiboot modules.
#define MULTIBOOT_MOD_ALIGN 0x00001000

//  Alignment of the multiboot info structure.
#define MULTIBOOT_INFO_ALIGN 0x00000008

//  Flags set in the 'flags' member of the multiboot header.

#define MULTIBOOT_TAG_ALIGN 8
#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_CMDLINE 1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME 2
#define MULTIBOOT_TAG_TYPE_MODULE 3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO 4
#define MULTIBOOT_TAG_TYPE_BOOTDEV 5
#define MULTIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_TAG_TYPE_VBE 7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS 9
#define MULTIBOOT_TAG_TYPE_APM 10
#define MULTIBOOT_TAG_TYPE_EFI32 11
#define MULTIBOOT_TAG_TYPE_EFI64 12
#define MULTIBOOT_TAG_TYPE_SMBIOS 13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD 14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW 15
#define MULTIBOOT_TAG_TYPE_NETWORK 16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP 17
#define MULTIBOOT_TAG_TYPE_EFI_BS 18
#define MULTIBOOT_TAG_TYPE_EFI32_IH 19
#define MULTIBOOT_TAG_TYPE_EFI64_IH 20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR 21

#define MULTIBOOT_HEADER_TAG_END 0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST 1
#define MULTIBOOT_HEADER_TAG_ADDRESS 2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS 3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS 4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER 5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN 6
#define MULTIBOOT_HEADER_TAG_EFI_BS 7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32 8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64 9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE 10

#define MULTIBOOT_ARCHITECTURE_I386 0
#define MULTIBOOT_ARCHITECTURE_MIPS32 4
#define MULTIBOOT_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT_LOAD_PREFERENCE_NONE 0
#define MULTIBOOT_LOAD_PREFERENCE_LOW 1
#define MULTIBOOT_LOAD_PREFERENCE_HIGH 2

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED 1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

// 声明这一段代码以 32 位模式编译
.code32

// multiboot2 文件头
// 计算头长度
.SET HEADER_LENGTH, multiboot_header_end - multiboot_header
// 计算校验和
.SET CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + HEADER_LENGTH)
// 8 字节对齐
.align MULTIBOOT_HEADER_ALIGN
// 声明所属段
.section .multiboot_header
multiboot_header:
    // 魔数
    .long MULTIBOOT2_HEADER_MAGIC
    // 架构
    .long MULTIBOOT_ARCHITECTURE_I386
    // 头长度
    .long HEADER_LENGTH
    // 校验和
    .long CHECKSUM
    // 添加其它内容在此，详细信息见 Multiboot2 Specification version 2.0.pdf
	.short MULTIBOOT_HEADER_TAG_END
    // 结束标记
    .short 0
    .long 8
multiboot_header_end:

// 声明所属段
.section .text
// 全局可见
.global _start
// 声明类型
.type _start, @function
// 声明外部定义
.extern kernel_main
.extern cpp_init
// 在 multiboot2.cpp 中定义
.extern boot_info_addr
.extern multiboot2_magic
_start:
    // 关中断
    cli
    // multiboot2_info 结构体指针
    mov %ebx, boot_info_addr
    // 魔数
    mov %eax, multiboot2_magic
    // 设置栈地址
    mov $STACK_TOP, %esp
    // 栈地址按照 16 字节对齐
    and $0xFFFFFFF0, %esp
    // 帧指针修改为 0
    mov $0, %ebp
    // 初始化 C++
    call cpp_init
    call kernel_main
    hlt
    ret

// 声明所属段
.section .bss
// 栈
STACK:
    // 跳过 16KB
    .skip 4096 * 4
STACK_TOP:

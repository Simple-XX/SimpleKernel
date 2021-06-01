
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// multiboot2.h for Simple-XX/SimpleKernel.

// 启动后，在 32 位内核进入点，机器状态如下：
//   1. CS 指向基地址为 0x00000000，限长为4G – 1的代码段描述符。
//   2. DS，SS，ES，FS 和 GS 指向基地址为0x00000000，限长为4G –
//   1的数据段描述符。
//   3. A20 地址线已经打开。
//   4. 页机制被禁止。
//   5. 中断被禁止。
//   6. EAX = 0x2BADB002
//   7. 系统信息和启动信息块的线性地址保存在 EBX中（相当于一个指针）。
//      以下即为这个信息块的结构

#ifndef _MULTIBOOT2_H_
#define _MULTIBOOT2_H_

#ifdef __cplusplus
extern "C" {
#endif

/*  How many bytes from the start of the file we search for the header. */
#define MULTIBOOT_SEARCH 32768
#define MULTIBOOT_HEADER_ALIGN 8

/*  The magic field should contain this. */
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

/*  This should be in %eax. */
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

/*  Alignment of multiboot modules. */
#define MULTIBOOT_MOD_ALIGN 0x00001000

/*  Alignment of the multiboot info structure. */
#define MULTIBOOT_INFO_ALIGN 0x00000008

/*  Flags set in the 'flags' member of the multiboot header. */

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

#ifdef __cplusplus
}
#endif

#endif /* _MULTIBOOT2_H_ */

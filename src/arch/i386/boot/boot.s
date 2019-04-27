
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
# boot.s for MRNIU/SimpleKernel.

# 3.1.1 The layout of Multiboot2 header
# The layout of the Multiboot2 header must be as follows:
# Offset Type Field Name        Note
# 0       u32   magic          required
# 4       u32   architecture   required
# 8       u32   header_length  required
# 12      u32   checksum       required
# 16-XX         tags           required

# multiboot2 定义
.set  MULTIBOOT_SEARCH,                        32768
.set  MULTIBOOT_HEADER_ALIGN,                  8
.set  MULTIBOOT2_HEADER_MAGIC,                 0xe85250d6
.set  MULTIBOOT2_BOOTLOADER_MAGIC,             0x36d76289
.set  MULTIBOOT_MOD_ALIGN,                     0x00001000
.set  MULTIBOOT_INFO_ALIGN,                    0x00000008

.set  MULTIBOOT_TAG_ALIGN,                  8
.set  MULTIBOOT_TAG_TYPE_END,               0
.set  MULTIBOOT_TAG_TYPE_CMDLINE,           1
.set  MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME,  2
.set  MULTIBOOT_TAG_TYPE_MODULE,            3
.set  MULTIBOOT_TAG_TYPE_BASIC_MEMINFO,     4
.set  MULTIBOOT_TAG_TYPE_BOOTDEV,           5
.set  MULTIBOOT_TAG_TYPE_MMAP,              6
.set  MULTIBOOT_TAG_TYPE_VBE,               7
.set  MULTIBOOT_TAG_TYPE_FRAMEBUFFER,       8
.set  MULTIBOOT_TAG_TYPE_ELF_SECTIONS,      9
.set  MULTIBOOT_TAG_TYPE_APM,               10
.set  MULTIBOOT_TAG_TYPE_EFI32,             11
.set  MULTIBOOT_TAG_TYPE_EFI64,             12
.set  MULTIBOOT_TAG_TYPE_SMBIOS,            13
.set  MULTIBOOT_TAG_TYPE_ACPI_OLD,          14
.set  MULTIBOOT_TAG_TYPE_ACPI_NEW,          15
.set  MULTIBOOT_TAG_TYPE_NETWORK,           16
.set  MULTIBOOT_TAG_TYPE_EFI_MMAP,          17
.set  MULTIBOOT_TAG_TYPE_EFI_BS,            18
.set  MULTIBOOT_TAG_TYPE_EFI32_IH,          19
.set  MULTIBOOT_TAG_TYPE_EFI64_IH,          20
.set  MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR,    21

.set  MULTIBOOT_HEADER_TAG_END,  0
.set  MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST,  1
.set  MULTIBOOT_HEADER_TAG_ADDRESS,  2
.set  MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS,  3
.set  MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS,  4
.set  MULTIBOOT_HEADER_TAG_FRAMEBUFFER,  5
.set  MULTIBOOT_HEADER_TAG_MODULE_ALIGN,  6
.set  MULTIBOOT_HEADER_TAG_EFI_BS,        7
.set  MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32,  8
.set  MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64,  9
.set  MULTIBOOT_HEADER_TAG_RELOCATABLE,  10

.set  MULTIBOOT_ARCHITECTURE_I386,  0
.set  MULTIBOOT_ARCHITECTURE_MIPS32,  4
.set  MULTIBOOT_HEADER_TAG_OPTIONAL, 1

.set  MULTIBOOT_LOAD_PREFERENCE_NONE, 0
.set  MULTIBOOT_LOAD_PREFERENCE_LOW, 1
.set  MULTIBOOT_LOAD_PREFERENCE_HIGH, 2

.set  MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED, 1
.set  MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED, 2

.section .multiboot_header
# multiboot2 文件头
.align 8
multiboot_header:
  .long MULTIBOOT2_HEADER_MAGIC
  .long MULTIBOOT_ARCHITECTURE_I386
  .long multiboot_header_end - multiboot_header
  .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + multiboot_header_end - multiboot_header)

# 添加其它内容在此，详细信息见 Multiboot2 Specification version 2.0.pdf

# multiboot2 information request
.align 8
mbi_tag_start:
  .short MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
  .short MULTIBOOT_HEADER_TAG_OPTIONAL
  .long mbi_tag_end - mbi_tag_start
  .long MULTIBOOT_TAG_TYPE_CMDLINE
  .long MULTIBOOT_TAG_TYPE_MODULE
  .long MULTIBOOT_TAG_TYPE_BOOTDEV
  .long MULTIBOOT_TAG_TYPE_MMAP
  .long MULTIBOOT_TAG_TYPE_ELF_SECTIONS
  .long MULTIBOOT_TAG_TYPE_APM
  .long MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR
.align 8
mbi_tag_end:
	.short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long 8
multiboot_header_end:

# Allocate the initial stack. 分配初始化栈
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
# 预分配用于开启分页的内存页。此地址不能硬编码
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

.section .text
.global start
.type start, @function
start:
  movl $(boot_page_table1 - 0xC0000000), %edi
  movl $0, %esi
  movl $1023, %ecx

1:
  # Only map the kernel.
	cmpl $(kernel_start - 0xC0000000), %esi
	jl 2f
	cmpl $(kernel_end - 0xC0000000), %esi
	jge 3f
  movl %esi, %edx
  orl $0x003, %edx
  movl %edx, (%edi)

2:
  # Size of page is 4096 bytes.
  addl $4096, %esi
  # Size of entries in boot_page_table1 is 4 bytes.
  addl $4, %edi
  # Loop to the next entry if we haven't finished.
  loop 1b

  3:
  # Map VGA video memory to 0xC03FF000 as "present, writable".
  movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

  # The page table is used at both page directory entry 0 (virtually from 0x0
  # to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
  # 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
  # higher half). The kernel is identity mapped because enabling paging does
  # not change the next instruction, which continues to be physical. The CPU
  # would instead page fault if there was no identity mapping.

  # Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
  movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
  movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

  # Set cr3 to the address of the boot_page_directory.
  movl $(boot_page_directory - 0xC0000000), %ecx
  movl %ecx, %cr3

  # Enable paging and the write-protect bit.
  movl %cr0, %ecx
  orl $0x80010000, %ecx
  movl %ecx, %cr0

  jmp multiboot_entry

multiboot_entry:
	# 设置栈地址
  mov $stack_top, %esp
  and $0xFFFFFFF0, %esp     # 栈地址按照 16 字节对齐
  mov $0, %ebp          # 帧指针修改为 0
  push $0
  popf
	# multiboot2_info 结构体指针
  push %ebx
	# 魔数
	push %eax
  call kernel_main
  cli
1:
  hlt
  jmp 1b
  ret

.size start, . - start

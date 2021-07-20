
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# boot.S for Simple-XX/SimpleKernel.

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


/* 直接用 -m64 编译出来的是 64 位代码，
    但是启动后的机器是 32 位的，相当于在 32 位机器上跑 64 位程序。
	得加一层跳转到 64 位的 -m32 代码，开启 long 模式后再跳转到以 -m64 编译的代码中
    对于 x86_64，需要在启动阶段进入长模式(IA32E)，这意味着需要一个临时页表
    See https://wiki.osdev.org/Creating_a_64-bit_kernel: 
	With a 32-bit bootstrap in your kernel
*/

# 这部分是从保护模式启动 long 模式的代码
# 工作在 32bit
.code32

# multiboot2 文件头
.SET HEADER_LENGTH, multiboot_header_end - multiboot_header
.SET CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + HEADER_LENGTH)
.align MULTIBOOT_HEADER_ALIGN
.section .multiboot_header
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

# 临时页表
.section .data
.align 0x1000
pml4:
    .skip 0x1000
pdpt:
    .skip 0x1000
pd:
    .skip 0x1000
pt:
    .skip 0x1000


.section .text
.global _start
.type _start, @function
_start:
    # 关中断
    cli
    # 从保护模式跳转到长模式
    # 1. 允许 PAE
    mov %cr4, %eax
    or $(1<<5), %eax
    mov %eax, %cr4
    # 2. 设置临时页表
    // TODO: 页表的设置有问题
    mov $pml4, %eax
    mov $pdpt, %ebx
    or $3, %ebx
    mov %ebx, 0(%eax)

    mov $pdpt, %eax
    mov $pd, %ebx
    or $3, %ebx
    mov %ebx, 0(%eax)

    mov $pd, %eax
    mov $pt, %ebx
    or $3, %ebx
    mov %ebx, 0(%eax)

    mov $pt, %eax
    mov $0x0003, %ebx
    mov %ebx, 0(%eax)
    mov $0x1003, %ebx
    mov %ebx, 1(%eax)
    mov $0x2003, %ebx
    mov %ebx, 2(%eax)
    mov $0x3003, %ebx
    mov %ebx, 3(%eax)
    mov $0x4003, %ebx
    mov %ebx, 4(%eax)
    mov $0x5003, %ebx
    mov %ebx, 5(%eax)
    mov $0x6003, %ebx
    mov %ebx, 6(%eax)
    mov $0x7003, %ebx
    mov %ebx, 7(%eax)
    mov $0x8003, %ebx
    mov %ebx, 8(%eax)


    # 填写 CR3
    mov $pml4, %eax
    mov %eax, %cr3

    # 3. 切换到 long 模式
    mov $0xC0000080, %ecx
    rdmsr
    or $(1<<8), %eax
    wrmsr
    # 4. 开启分页
    mov %cr0, %eax
    or $(1<<31), %eax
    mov %eax, %cr0
    hlt
    # 5. 重新设置 GDT
    # 6. 跳转到 64 位代码执行
    jmp _start64
    hlt
    ret

.code64

.section .text
.global _start64
.type _start64, @function
.extern kernel_main
_start64:
    # 设置栈地址
    mov $STACK_TOP, %rsp
    # 栈地址按照 4096 字节对齐
    and $0xFFFFFFFFFFFFF000, %rsp
    # 帧指针修改为 0
    mov $0, %rbp
    push $0
    popf
	# multiboot2_info 结构体指针
    push %rbx
	# 魔数
	push %rax
    call kernel_main
    hlt
    ret

.section .bss
STACK:
    .skip 16384
STACK_TOP:

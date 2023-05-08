
/**
 * @file boot.S
 * @brief 启动代码，进行一些设置后跳转到 kernel_main
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-01-02<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

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


// 直接用 -m64 编译出来的是 64 位代码，
// 但是启动后的机器是 32 位的，相当于在 32 位机器上跑 64 位程序。
// 得加一层跳转到 64 位的 -m32 代码，开启 long 模式后再跳转到以 -m64 编译的代码中
// 对于 x86_64，需要在启动阶段进入长模式(IA32E)，这意味着需要一个临时页表
// See https://wiki.osdev.org/Creating_a_64-bit_kernel: 
// With a 32-bit bootstrap in your kernel

// 这部分是从保护模式启动 long 模式的代码
// 工作在 32bit
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
.section .text.boot
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

// 临时页表 4KB/页
.section .data.boot
.align 0x1000
pml4:
    .skip 0x1000
pdpt:
    .skip 0x1000
pd:
    .skip 0x1000
pt:
    .skip 0x1000

// 临时 GDT
.align 16
gdt64:
null_desc:
    .short 0xFFFF
    .short 0
    .byte 0
    .byte 0
    .byte 0
    .byte 0
code_desc:
    .short 0
    .short 0
    .byte 0
    .byte 0x9A
    .byte 0x20
    .byte 0
data_desc:
    .short 0
    .short 0
    .byte 0
    .byte 0x92
    .byte 0
    .byte 0
user_code_desc:
    .short 0
    .short 0
    .byte 0
    .byte 0xFA
    .byte 0x20
    .byte 0
user_data_desc:
    .short 0
    .short 0
    .byte 0
    .byte 0xF2
    .byte 0
    .byte 0
gdt64_pointer:
    .short gdt64_pointer-gdt64-1
    .quad gdt64
gdt64_pointer64:
    .short gdt64_pointer-gdt64-1
    .quad gdt64

.section .text.boot
.global _start
.type _start, @function
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
    // 从保护模式跳转到长模式
    // 1. 允许 PAE
    mov %cr4, %eax
    or $(1<<5), %eax
    mov %eax, %cr4
    // 2. 设置临时页表
    // 最高级
    mov $pml4, %eax
    mov $pdpt, %ebx
    or $0x3, %ebx
    mov %ebx, 0(%eax)
    // 次级
    mov $pdpt, %eax
    mov $pd, %ebx
    or $0x3, %ebx
    mov %ebx, 0(%eax)
    // 次低级
    mov $pd, %eax
    mov $pt, %ebx
    or $0x3, %ebx
    mov %ebx, 0(%eax)
    // 最低级
    // 循环 512 次，填满一页
    mov $512, %ecx
    mov $pt, %eax
    mov $0x3, %ebx
.fill_pt:
    mov %ebx, 0(%eax)
    add $0x1000, %ebx
    add $8, %eax
    loop .fill_pt
    // 填写 CR3
    mov $pml4, %eax
    mov %eax, %cr3
    // 3. 切换到 long 模式
    mov $0xC0000080, %ecx
    rdmsr
    or $(1<<8), %eax
    wrmsr
    // 4. 开启分页
    mov %cr0, %eax
    or $(1<<31), %eax
    mov %eax, %cr0
    // 5. 重新设置 GDT
    mov $gdt64_pointer, %eax
    lgdt 0(%eax)
    // 6. 跳转到 64 位代码执行
    jmp $0x8, $_start64
    hlt
    ret

.code64
.section .text.boot
.global _start64
.type _start64, @function
.extern kernel_main
.extern cpp_init
_start64:
    // 从这里开始就是 long 模式了
    // 加载 64 位 gdt
    mov $gdt64_pointer64, %rax
    lgdt 0(%rax)
    // 更新
    mov $0x10, %rax
    mov %rax, %ds
    mov %rax, %es
    mov %rax, %fs
    mov %rax, %gs
    mov %rax, %ss
    // 设置栈地址
    mov $stack_top, %rsp
    // 栈地址按照 4096 字节对齐
    and $0xFFFFFFFFFFFFF000, %rsp
    // 帧指针修改为 0
    mov $0, %rbp
    // 初始化 C++
    call cpp_init
    call kernel_main
    hlt
    ret

// 声明所属段
.section .bss.boot
// 16 字节对齐
.align 16
.global stack_top
// 栈
stack_top:
    // 跳过 16KB
    .space 4096 * 4

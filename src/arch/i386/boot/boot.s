
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
# boot.s for MRNIU/SimpleKernel.

/* Declare constants for the multiboot header. */

/*  How many bytes from the start of the file we search for the header. */
.set MULTIBOOT_SEARCH,                        32768
.set MULTIBOOT_HEADER_ALIGN,                  8

/*  The magic field should contain this. */
.set MULTIBOOT2_HEADER_MAGIC,                 0xe85250d6

/*  This should be in %eax. */
.set MULTIBOOT2_BOOTLOADER_MAGIC,             0x36d76289

/*  Alignment of multiboot modules. */
.set MULTIBOOT_MOD_ALIGN,                     0x00001000

/*  Alignment of the multiboot info structure. */
.set MULTIBOOT_INFO_ALIGN,                    0x00000008

/*  Flags set in the 'flags' member of the multiboot header. */

.set MULTIBOOT_TAG_ALIGN,                  8
.set MULTIBOOT_TAG_TYPE_END,               0
.set MULTIBOOT_TAG_TYPE_CMDLINE,           1
.set MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME,  2
.set MULTIBOOT_TAG_TYPE_MODULE,            3
.set MULTIBOOT_TAG_TYPE_BASIC_MEMINFO,     4
.set MULTIBOOT_TAG_TYPE_BOOTDEV,           5
.set MULTIBOOT_TAG_TYPE_MMAP,              6
.set MULTIBOOT_TAG_TYPE_VBE,               7
.set MULTIBOOT_TAG_TYPE_FRAMEBUFFER,       8
.set MULTIBOOT_TAG_TYPE_ELF_SECTIONS,      9
.set MULTIBOOT_TAG_TYPE_APM,               10
.set MULTIBOOT_TAG_TYPE_EFI32,             11
.set MULTIBOOT_TAG_TYPE_EFI64,             12
.set MULTIBOOT_TAG_TYPE_SMBIOS,            13
.set MULTIBOOT_TAG_TYPE_ACPI_OLD,          14
.set MULTIBOOT_TAG_TYPE_ACPI_NEW,          15
.set MULTIBOOT_TAG_TYPE_NETWORK,           16
.set MULTIBOOT_TAG_TYPE_EFI_MMAP,          17
.set MULTIBOOT_TAG_TYPE_EFI_BS,            18
.set MULTIBOOT_TAG_TYPE_EFI32_IH,          19
.set MULTIBOOT_TAG_TYPE_EFI64_IH,          20
.set MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR,    21

.set MULTIBOOT_HEADER_TAG_END,  0
.set MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST,  1
.set MULTIBOOT_HEADER_TAG_ADDRESS,  2
.set MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS,  3
.set MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS,  4
.set MULTIBOOT_HEADER_TAG_FRAMEBUFFER,  5
.set MULTIBOOT_HEADER_TAG_MODULE_ALIGN,  6
.set MULTIBOOT_HEADER_TAG_EFI_BS,        7
.set MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32,  8
.set MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64,  9
.set MULTIBOOT_HEADER_TAG_RELOCATABLE,  10

.set MULTIBOOT_ARCHITECTURE_I386,  0
.set MULTIBOOT_ARCHITECTURE_MIPS32,  4
.set MULTIBOOT_HEADER_TAG_OPTIONAL, 1

.set MULTIBOOT_LOAD_PREFERENCE_NONE, 0
.set MULTIBOOT_LOAD_PREFERENCE_LOW, 1
.set MULTIBOOT_LOAD_PREFERENCE_HIGH, 2

.set MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED, 1
.set MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED, 2


/*  The size of our stack (16KB). */
.set STACK_SIZE,                      0x4000

/*  The flags for the Multiboot header. */
#ifdef __ELF__
# define AOUT_KLUDGE 0
#else
# define AOUT_KLUDGE MULTIBOOT_AOUT_KLUDGE
#endif
.set AOUT_KLUDGE, 0

.text
.globl  start, _start


start:
_start:
	jmp multiboot_entry
	/*  Align 64 bits boundary. */
  .align  8

  /*  Multiboot header. */
multiboot_header:
  /*  magic */
  .long   MULTIBOOT2_HEADER_MAGIC
  /*  ISA: i386 */
  .long   GRUB_MULTIBOOT_ARCHITECTURE_I386
  /*  Header length. */
  .long   multiboot_header_end - multiboot_header
  /*  checksum */
  .long   -(MULTIBOOT2_HEADER_MAGIC + GRUB_MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

#ifndef __ELF__
#address_tag_start:
#  .short MULTIBOOT_HEADER_TAG_ADDRESS
#  .short MULTIBOOT_HEADER_TAG_OPTIONAL
#  .long address_tag_end - address_tag_start
#  /*  header_addr */
#  .long   multiboot_header
#  /*  load_addr */
#  .long   _start
#  /*  load_end_addr */
#  .long   _edata
#  /*  bss_end_addr */
#  .long   _end
#address_tag_end:
#entry_address_tag_start:
#  .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
#  .short MULTIBOOT_HEADER_TAG_OPTIONAL
#  .long entry_address_tag_end - entry_address_tag_start
#  /*  entry_addr */
#  .long multiboot_entry
#entry_address_tag_end:
#endif /*  __ELF__ */

framebuffer_tag_start:
  .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
  .short MULTIBOOT_HEADER_TAG_OPTIONAL
  .long framebuffer_tag_end - framebuffer_tag_start
  .long 1024
  .long 768
  .long 32
framebuffer_tag_end:
  .short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long 8
multiboot_header_end:
multiboot_entry:
  /*  Initialize the stack pointer. */
  movl    $(stack + STACK_SIZE), %esp

  /*  Reset EFLAGS. */
  pushl   $0
  popf

  /*  Push the pointer to the Multiboot information structure. */
  pushl   %ebx
  /*  Push the magic value. */
  pushl   %eax

  /*  Now enter the C main function... */
  call    kernel_main

  /*  Halt. */
  pushl   $halt_message
  call    printk

loop:   hlt
  jmp     loop

halt_message:
  .asciz  "Halted."

  /*  Our stack area. */
.comm   stack, STACK_SIZE

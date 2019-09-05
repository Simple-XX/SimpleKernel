/* Based on osdev.org Bare_Bones:*/
/* (https://wiki.osdev.org/Bare_Bones).*/

/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link.ld for MRNIU/SimpleKernel.*/

OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS
{
	/* 指定内核从地址 1M 处开始 */
	. = 1M;

    PROVIDE(kernel_start = .);

	PROVIDE(multiboot_start = .);
	.multiboot	:
	{
		*(.multiboot_header)
	}
	PROVIDE(multiboot_end = .);
	PROVIDE(kernel_text_start = .);
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}
  PROVIDE(kernel_text_end = .);

  PROVIDE(kernel_rodata_start = .);
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

	PROVIDE(kernel_data_start = .);
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
  PROVIDE(kernel_data_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = .);

  PROVIDE(kernel_end = .);
}

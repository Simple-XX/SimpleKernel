/* Based on osdev.org Bare_Bones:*/
/* (https://wiki.osdev.org/Bare_Bones).*/

/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link.ld for MRNIU/SimpleKernel.*/

ENTRY(start)

SECTIONS
{
	/* 指定内核从地址 1M 处开始 */
	. = 1M;

  PROVIDE(kernel_start = .);
  PROVIDE(kernel_text_start = .);

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot_header)
		*(.text)
	}
  PROVIDE(kernel_text_end = .);

  PROVIDE(kernel_data_start = .);
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
  PROVIDE(kernel_data_end = .);

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

  PROVIDE(kernel_end = .);

}

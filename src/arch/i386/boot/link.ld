/* Based on osdev.org Bare_Bones:*/
/* (https://wiki.osdev.org/Bare_Bones).*/

/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link.ld for MRNIU/SimpleKernel.*/

OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS
{
  PROVIDE(kernel_start = 0xC0000000);

	. = 1M;
	PROVIDE(multiboot_start = .);
	.multiboot ALIGN (4K) :
	{
		*(.multiboot_header)
	}
	PROVIDE(multiboot_end = .);

	. += 0xC0000000;
	PROVIDE(kernel_text_start = .);
	.text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.text)
	}
  PROVIDE(kernel_text_end = .);

  PROVIDE(kernel_rodata_start = .);
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

	PROVIDE(kernel_data_start = .);
	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}
  PROVIDE(kernel_data_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	PROVIDE(kernel_bss_end = .);

  PROVIDE(kernel_end = .);
}

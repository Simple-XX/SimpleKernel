/* Based on osdev.org Bare_Bones:*/
/* (https://wiki.osdev.org/Bare_Bones).*/

/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link.ld for MRNIU/SimpleKernel.*/

ENTRY(_start)

SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;

  PROVIDE(kernel_start = .);
  PROVIDE(kernel_text_start = .);

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
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
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}


/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT(elf32-littlearm)

ENTRY(_start)

SECTIONS
{
    /* 加载地址 */
    . = 0x8000;
	
	kernel_start = .;
    kernel_text_start = .;
    .text : ALIGN(4K)
    {
        *.(.text.init)
        *(.text)
    }
    kernel_text_end = .;

    kernel_rodata_start = .;
	.rodata : ALIGN(4K)
	{
		*(.rodata)
	}
	kernel_rodata_end = .;

    kernel_data_start = .;
    .data : ALIGN(4K)
    {
        ctors_start = .;
	    *(.ctor*)
	    ctors_end = .;
	    dtors_start = .;
	    *(.dtor*)
	    dtors_end = .;
        *(.data)
    }
    kernel_data_end = .;

    kernel_bss_start = .;
	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	kernel_bss_end = .;

	kernel_end = .;
}

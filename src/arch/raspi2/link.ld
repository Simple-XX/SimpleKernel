
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link32.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT(elf32-littlearm)

ENTRY(_start)

SECTIONS
{
    /* 加载地址 */
    . = 0x8000;
	
	PROVIDE(kernel_start = .);
    PROVIDE(kernel_text_start = .);
    .text : ALIGN(4K)
    {
        *.(.text.init)
        *(.text)
    }
    PROVIDE(kernel_text_end = .);

    PROVIDE(kernel_data_start = .);
    .data : ALIGN(4K)
    {
        PROVIDE(ctors_start = .);
	    *(.ctor*)
	    PROVIDE(ctors_end = .);
	    PROVIDE(dtors_start = .);
	    *(.dtor*)
	    PROVIDE(dtors_end = .);
        *(.data)
    }
    PROVIDE(kernel_data_end = .);
 
    PROVIDE(kernel_rodata_start = .);
	.rodata : ALIGN(4K)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

    PROVIDE(kernel_bss_start = .);
	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = .);

	PROVIDE(kernel_end = .);
}

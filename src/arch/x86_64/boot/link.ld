
/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link.ld for MRNIU/SimpleKernel.*/


OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

SECTIONS
{

    /* 指定内核从地址 1M 处开始 */
    . = 1M;
    PROVIDE(kernel_start = .);

    PROVIDE(init_start = .);
	.init.text : ALIGN(4K)
	{
		*(.multiboot_header)
        *(.init.text)
	}
    .init.data : ALIGN(4K)
	{
        *(.init.data)
        *(.init.bss)
	}
	PROVIDE(init_end = .);

	PROVIDE(kernel_text_start = .);
	.text : ALIGN(4K)
	{
		*(.text)
	}
    PROVIDE(kernel_text_end = .);

    PROVIDE(kernel_rodata_start = .);
	.rodata : ALIGN(4K)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

	PROVIDE(kernel_data_start = .);
	.data : ALIGN(4K)
	{
		*(.data)
	}
    PROVIDE(kernel_data_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = .);

    PROVIDE(kernel_end = .);
}

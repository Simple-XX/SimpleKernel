
/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link32.ld for MRNIU/SimpleKernel.*/

OUTPUT_FORMAT(elf32-i386)

ENTRY(start)

SECTIONS
{

    /* 指定内核从地址 1M 处开始 */
    . = 0xC0100000;
    PROVIDE(kernel_start = .);

    PROVIDE(init_start = .);
	.init.text ALIGN(4K) : AT(ADDR (.init.text) - 0xC0000000)
	{
		*(.multiboot_header)
        *(.init.text)
	}
    .init.data ALIGN(4K) : AT(ADDR (.init.data) - 0xC0000000)
	{
        *(.init.data)
        *(.init.bss)
	}
	PROVIDE(init_end = .);

	PROVIDE(kernel_text_start = .);
	.text ALIGN(4K) : AT(ADDR (.text) - 0xC0000000)
	{
		*(.text)
	}
    PROVIDE(kernel_text_end = .);

    PROVIDE(kernel_rodata_start = .);
	.rodata ALIGN(4K) : AT(ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

	PROVIDE(kernel_data_start = .);
	.data ALIGN(4K) : AT(ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}
    PROVIDE(kernel_data_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss ALIGN(4K) : AT(ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = .);

    PROVIDE(kernel_end = .);
}

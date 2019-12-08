
/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/

/* link32.ld for MRNIU/SimpleKernel.*/

OUTPUT_FORMAT(elf32-i386)

ENTRY(_start)

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

    . += 0xC0000000;

	PROVIDE(kernel_text_start = . - 0xC0000000);
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}
    PROVIDE(kernel_text_end = . - 0xC0000000);

    PROVIDE(kernel_data_start = . - 0xC0000000);
    .data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }
    PROVIDE(kernel_data_end = . - 0xC0000000);

    PROVIDE(kernel_rodata_start = . - 0xC0000000);
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = . - 0xC0000000);

	PROVIDE(kernel_bss_start = . - 0xC0000000);
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = . - 0xC0000000);

    PROVIDE(kernel_end = . - 0xC0000000);
}

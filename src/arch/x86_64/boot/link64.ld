
/* This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
 *
 * link32.ld for SimpleXX/SimpleKernel. */

OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS
{
    /* VMA 为顺序排列，LMA 按照 AT(addr) 排列 */
    /* 指定内核从地址 1M 处开始 */
    . = 1M;

    PROVIDE(kernel_init_start = .);
    PROVIDE(kernel_init_text_start = .);
	.init.text : ALIGN(4K)
	{
		*(.multiboot_header)
        *(.init.text)
	}
    PROVIDE(kernel_init_text_end = .);
    PROVIDE(kernel_init_data_start = .);
    .init.data : ALIGN(4K)
	{
        *(.init.data)
        *(.init.bss)
	}
    PROVIDE(kernel_init_data_end = .);
	PROVIDE(kernel_init_end = .);

    . += 0xC0000000;

    PROVIDE(kernel_start = .);
    PROVIDE(kernel_text_start = .);
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}
    PROVIDE(kernel_text_end = .);

    PROVIDE(kernel_data_start = .);
    .data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }
    PROVIDE(kernel_data_end = .);

    PROVIDE(kernel_rodata_start = .);
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = .);

    PROVIDE(kernel_end = .);
}

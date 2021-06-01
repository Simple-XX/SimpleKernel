
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT(elf32-i386)

ENTRY(_start)

SECTIONS {
    /* VMA 为顺序排列，LMA 按照 AT(addr) 排列 */
    /* 指定内核从地址 1M 处开始 */
    . = 1M;
    PROVIDE(kernel_start = .);

	.text : ALIGN(4K) {
		PROVIDE(kernel_text_start = .);
		*(.multiboot_header)
		*(.text)
		PROVIDE(kernel_text_end = .);
	}

	.ctors : ALIGN(4K) {
		PROVIDE (ctors_start = .);
        *(SORT_BY_INIT_PRIORITY (.init_array.*))
		*(SORT_BY_INIT_PRIORITY (.ctors.*))
        *(.init_array .ctors)
		PROVIDE (ctors_end = .);
    }

	.rodata : ALIGN(4K) {
		PROVIDE(kernel_rodata_start = .);
		*(.rodata)
		PROVIDE(kernel_rodata_end = .);
	}

    .data : ALIGN(4K) {
		PROVIDE(kernel_data_start = .);
        *(.data)
		PROVIDE(kernel_data_end = .);
    }

	.bss : ALIGN(4K) {
		PROVIDE(kernel_bss_start = .);
		*(COMMON)
		*(.bss)
		PROVIDE(kernel_bss_end = .);
	}

    PROVIDE(kernel_end = .);
}

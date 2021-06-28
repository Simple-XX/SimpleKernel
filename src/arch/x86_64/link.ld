
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS {
    . = 4M;
    PROVIDE(kernel_start = .);

    .text : ALIGN(4K) {
		PROVIDE(kernel_text_start = .);
		*(.multiboot_header)
		*(.text*)
		PROVIDE(kernel_text_end = .);
	}

	.ctors :  ALIGN(4K) {
		PROVIDE (ctors_start = .);
        *(SORT_BY_INIT_PRIORITY (.init_array.*))
		*(SORT_BY_INIT_PRIORITY (.ctors.*))
        *(.init_array .ctors)
		PROVIDE (ctors_end = .);
    }

	.rodata : ALIGN(4K) {
		PROVIDE(kernel_rodata_start = .);
		*(.rodata*)
		*(.gcc_except_table)
		PROVIDE(kernel_rodata_end = .);
	}

    .data : ALIGN(4K) {
		PROVIDE(kernel_data_start = .);
        *(.data*)
		*(.eh_frame)
		PROVIDE(kernel_data_end = .);
    }

	.bss : ALIGN(4K) {
		PROVIDE(kernel_bss_start = .);
		*(.bss)
		PROVIDE(kernel_bss_end = .);
	}

	.debug : ALIGN(4K) {
		PROVIDE(kernel_debug_start = .);
		*(.debug*)
		PROVIDE(kernel_debug_end = .);
	}

    PROVIDE(kernel_end = .);
}

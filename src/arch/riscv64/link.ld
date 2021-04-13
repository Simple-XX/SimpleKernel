
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH("riscv")

ENTRY(_start)

SECTIONS {
	. = 0x80200000;

    PROVIDE(kernel_start = .);
    PROVIDE(kernel_text_start = .);
	.text : ALIGN(4K) {
		*(.init)
		*(.text)
	}
    PROVIDE(kernel_text_end = .);

    PROVIDE(kernel_rodata_start = .);
	.rodata : ALIGN(4K) {
		*(.rodata)
	}
	PROVIDE(kernel_rodata_end = .);

    PROVIDE(kernel_data_start = .);
    .data : ALIGN(4K) {
		PROVIDE(ctors_start = .);
	    *(.ctor*)
	    PROVIDE(ctors_end = .);
	    PROVIDE(dtors_start = .);
	    *(.dtor*)
	    PROVIDE(dtors_end = .);
        *(.data)
    }
    PROVIDE(kernel_data_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss : ALIGN(4K) {
		*(.bss);
		. += 4096;
		PROVIDE(sqtack_top = .);
		. += 4096;
		PROVIDE(global_pointer = .);
	}
	PROVIDE(kernel_bss_end = .);

    PROVIDE(kernel_end = .);
}

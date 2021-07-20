
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS {
	/* TODO: 这里似乎可以修改为任意地址 */
    . = 4M;

    PROVIDE(kernel_start = .);
    PROVIDE(kernel_text_start = .);
	.text : ALIGN(4K) {
		*(.multiboot_header)
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
        *(.data)
    }
    PROVIDE(kernel_data_end = .);

	PROVIDE(kernel_bss_start = .);
	.bss : ALIGN(4K) {
		*(COMMON)
		*(.bss)
	}
	PROVIDE(kernel_bss_end = .);
    PROVIDE(kernel_end = .);
}

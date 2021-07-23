
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)

ENTRY(_start)

SECTIONS {
	/* 设置起始地址 */
    . = 0x80200000;
    PROVIDE(kernel_start = .);

    PROVIDE(kernel_text_start = .);
	/* 代码段 */
    .text : ALIGN(4K) {
        *(.init)
        *(.text*)
    }
    PROVIDE(kernel_text_end = .);

    PROVIDE(kernel_rodata_start = .);
	/* 只读数据段 */
    .rodata : ALIGN(4K) {
        PROVIDE(ctors_start = .);
        *(SORT_BY_INIT_PRIORITY (.init_array.*))
        *(SORT_BY_INIT_PRIORITY (.ctors.*))
        *(.init_array .ctors)
        PROVIDE(ctors_end = .);
        PROVIDE(dtors_start = .);
        *(.dtor*)
        PROVIDE(dtors_end = .);
        *(.rodata*)
        *(.gcc_except_table)
        *(.riscv.attributes)
    }
    PROVIDE(kernel_rodata_end = .);

    PROVIDE(kernel_data_start = .);
	/* 数据段 */
    .data : ALIGN(4K) {
        *(.data*)
        *(.eh_frame)
        *(.got*)
    }
    PROVIDE(kernel_data_end = .);

    PROVIDE(kernel_bss_start = .);
	/* 未初始化数据段 */
    .bss : ALIGN(4K) {
        *(.bss);
    }
    PROVIDE(kernel_bss_end = .);

    PROVIDE(kernel_debug_start = .);
	/* 调试信息 */
    .debug : ALIGN(4K) {
        *(.debug*)
    }
    PROVIDE(kernel_debug_end = .);

    PROVIDE(kernel_end = .);
}

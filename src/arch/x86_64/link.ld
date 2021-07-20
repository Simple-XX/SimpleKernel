
/* This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

 /* TODO: 直接用 -m64 编译出来的是 64 位代码，
 	但是启动后的机器是 32 位的，相当于在 32 位机器上跑 64 位程序。
	得加一层跳转到 64 位的 -m32 代码，开启 long 模式后再跳转到以 -m64 编译的代码中
*/

ENTRY(_start)

SECTIONS {
	/* TODO: 这里似乎可以修改为任意地址 */
    . = 4M;

    PROVIDE(kernel_start = .);

    PROVIDE(kernel_text_start = .);
	/* 代码段 */
    .text : ALIGN(4K) {
        *(.multiboot_header)
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

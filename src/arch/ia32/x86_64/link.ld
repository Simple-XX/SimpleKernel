
/* This file is a part of Simple-XX/SimpleKernel
 * (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel. 
 * 链接脚本，指定生成的二进制文件的布局 */

/* 指定输出格式 */
OUTPUT_FORMAT(elf64-x86-64)
/* 执行输出架构 */
OUTPUT_ARCH(i386:x86-64)
/* 设置入口点 */
ENTRY(_start)
/* 设置各个 section */
SECTIONS {
    . = 0;

    PROVIDE(kernel_start = .);
    /* 指定内核从地址 1M 处开始 */
    /* 0~1M 的空间为 BIOS 保留区域 */
    /* TODO: 这里似乎可以修改为任意地址 */
    . = 1M;

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
        /* 构造函数起点 */
        PROVIDE(ctors_start = .);
        *(SORT_BY_INIT_PRIORITY (.init_array.*))
        *(SORT_BY_INIT_PRIORITY (.ctors.*))
        *(.init_array .ctors)
        /* 构造函数终点 */
        PROVIDE(ctors_end = .);
        /* 析构函数起点 */
        PROVIDE(dtors_start = .);
        *(.dtor*)
        /* 析构函数终点 */
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

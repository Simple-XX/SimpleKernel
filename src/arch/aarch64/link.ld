
/* This file is a part of Simple-XX/SimpleKernel
 * (https://github.com/Simple-XX/SimpleKernel).
 *
 * link.ld for Simple-XX/SimpleKernel.
 * 链接脚本，指定生成的二进制文件的布局
 */

OUTPUT_FORMAT(elf64-littleaarch64)

ENTRY(_start)

SECTIONS
{
    /* 加载地址 */
    . = 0x8000;
	
	kernel_start = .;
    kernel_text_start = .;
    .text : ALIGN(4K)
    {
        *.(.text.init)
        *(.text)
    }
    kernel_text_end = .;

    kernel_rodata_start = .;
	.rodata : ALIGN(4K)
	{
		*(.rodata)
	}
	kernel_rodata_end = .;

    kernel_data_start = .;
    .data : ALIGN(4K)
    {
        ctors_start = .;
	    *(.ctor*)
	    ctors_end = .;
	    dtors_start = .;
	    *(.dtor*)
	    dtors_end = .;
        *(.data)
    }
    kernel_data_end = .;

    kernel_bss_start = .;
	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	kernel_bss_end = .;

	kernel_end = .;
}

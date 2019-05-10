/* Based on osdev.org Bare_Bones:*/
/* (https://wiki.osdev.org/Bare_Bones).*/

/* This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).*/
/* link.ld for MRNIU/SimpleKernel.*/

OUTPUT_FORMAT(elf32-i386)

ENTRY(start)
SECTIONS
{
    PROVIDE( kernel_start = 0xC0100000 );

    . = 0x100000;

    PROVIDE( multiboot_start = . );
    .init.multiboot ALIGN (4K) :
    {
      *(.multiboot_header)
    }
    PROVIDE( multiboot_end = . );

    PROVIDE( init_text_start = . );
    .init.text ALIGN (4K) :
    {
        *(.init.text)
    }
    PROVIDE( init_text_end = . );

    PROVIDE( init_data_start = . );
    .init.data ALIGN (4K) :
    {
        *(.init.data)
    }
    PROVIDE( init_data_end = . );

    . += 0xC0000000;
    PROVIDE( kernel_text_start = . );
    .text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
    {
        *(.text)
    }
    PROVIDE( kernel_text_end = . );

    PROVIDE( kernel_data_start = . );
    .data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data)
        *(.rodata)
    }
    PROVIDE( kernel_data_end = . );

    PROVIDE( kernel_bss_start = . );
    .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
    }
    PROVIDE( kernel_bss_end = . );

    PROVIDE( kernel_stab_start = . );
    .stab ALIGN (4K) : AT(ADDR(.stab) - 0xC0000000)
    {
        *(.stab)
    }
    PROVIDE( kernel_stab_end = .);

    PROVIDE( kernel_stabstr_start = . );
    .stabstr ALIGN (4K) : AT(ADDR(.stabstr) - 0xC0000000)
    {
        *(.stabstr)
    }
    PROVIDE( kernel_stabstr_end = . );

    PROVIDE( kernel_end = . );

    /DISCARD/ : { *(.eh_frame) }
}

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-bigaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_boot)

SECTIONS {
    . = 0x40100000;

    .boot ALIGN(0x1000) : {
        *(.text.boot)
        *(.data.boot)
        *(.bss.boot)
    }

    .text ALIGN(0x1000) : {
        *(.text .text.*)
    }

    .rodata ALIGN(0x1000) : {
        *(.rodata .rodata.*)
    }

    .data ALIGN(0x1000) : {
        *(.data .data.*)
    }

    .bss ALIGN(0x1000) : {
        *(.bss .bss.*)
        *(COMMON)
    }

    _end = .;

    /DISCARD/ : {
        *(.note.GNU-stack)
        *(.comment)
    }
}

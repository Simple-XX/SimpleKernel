OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_boot)

SECTIONS {
    . = 0x80200000;

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

    .got ALIGN(0x1000) : {
        *(.got .got.*)
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

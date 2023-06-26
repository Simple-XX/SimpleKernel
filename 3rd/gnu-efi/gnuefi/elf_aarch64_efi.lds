OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
  . = 0;
  ImageBase = .;
  /* .hash and/or .gnu.hash MUST come first! */
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  . = ALIGN(4096);
  .eh_frame : { *(.eh_frame) }
  .gcc_except_table : { *(.gcc_except_table*) }
  . = ALIGN(4096);
  .text : {
    _text = .;
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
    *(.srodata)
    *(.rodata*)
    . = ALIGN(16);
  }
  _etext = .;
  _text_size = . - _text;
  . = ALIGN(4096);
  .reloc :
  {
    KEEP (*(.reloc))
  }
  . = ALIGN(65536);
  .dynamic  : { *(.dynamic) }
  . = ALIGN(4096);
  .data :
  {
   _data = .;
   *(.sdata)
   *(.data)
   *(.data1)
   *(.data.*)
   *(.got.plt)
   *(.got)

   /*
    * Note that these aren't the using the GNU "CONSTRUCTOR" output section
    * command, so they don't start with a size.  Because of p2align and the
    * end/END definitions, and the fact that they're mergeable, they can also
    * have NULLs which aren't guaranteed to be at the end.
    */
   . = ALIGN(16);
   _init_array = .;
   *(SORT_BY_NAME(.init_array))
   _init_array_end = .;
   __CTOR_LIST__ = .;
   *(SORT_BY_NAME(.ctors))
   __CTOR_END__ = .;
   __DTOR_LIST__ = .;
   *(SORT_BY_NAME(.dtors))
   __DTOR_END__ = .;
   _fini_array = .;
   *(SORT_BY_NAME(.fini_array))
   _fini_array_end = .;

   /* the EFI loader doesn't seem to like a .bss section, so we stick
      it all into .data: */
   . = ALIGN(16);
   _bss = .;
   *(.sbss)
   *(.scommon)
   *(.dynbss)
   *(.bss*)
   *(COMMON)
   *(.rel.local)
   . = ALIGN(16);

   _bss_end = .;
  }

  . = ALIGN(4096);
  .rela :
  {
    *(.rela.text*)
    *(.rela.data*)
    *(.rela.got)
    *(.rela.dyn)
    *(.rela.stab)
  }
  . = ALIGN(4096);
  .rela.plt : { *(.rela.plt) }
  . = ALIGN(512);
  _edata = .;
  _data_size = . - _data;

  . = ALIGN(4096);
  .dynsym   : { *(.dynsym) }
  . = ALIGN(4096);
  .dynstr   : { *(.dynstr) }
  . = ALIGN(4096);
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  .ignored.reloc :
  {
    *(.rela.reloc)
    *(.note.GNU-stack)
  }
  .comment 0 : { *(.comment) }
}

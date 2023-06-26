OUTPUT_FORMAT("elf64-ia64-little")
OUTPUT_ARCH(ia64)
ENTRY(_start_plabel)
SECTIONS
{
  . = 0;
  ImageBase = .;
  /* .hash and/or .gnu.hash MUST come first! */
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .eh_frame : { *(.eh_frame) }
  .gcc_except_table : { *(.gcc_except_table*) }
  . = ALIGN(4096);
  .text :
  {
   _text = .;
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
   . = ALIGN(16);
  }
  _etext = .;
  _text_size = . - _text;
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  __gp = ALIGN (8) + 0x200000;
  .sdata :
  {
   _data = .;
   *(.got.plt)
   *(.got)
   *(.srodata)
   *(.sdata)
   *(.sbss)
   *(.scommon)
  }
  . = ALIGN(4096);
  .data :
  {
   *(.rodata*)
   *(.ctors)
   *(.data*)
   *(.gnu.linkonce.d*)
   *(.plabel)	/* data whose relocs we want to ignore */

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
   *(.dynbss)
   *(.bss*)
   *(COMMON)
  }
  .note.gnu.build-id : { *(.note.gnu.build-id) }

  . = ALIGN(4096);
  .dynamic  : { *(.dynamic) }
  . = ALIGN(4096);
  .rela :
  {
    *(.rela.text*)
    *(.rela.sdata*)
    *(.rela.data*)
    *(.rela.got)
    *(.rela.dyn)
    *(.rela.gnu.linkonce.d*)
    *(.rela.stab)
    *(.rela.ctors)
  }
  . = ALIGN(4096);
  .rela.plt : { *(.rela.plt) }
  _edata = .;
  _data_size = . - _etext;
  . = ALIGN(4096);
  .reloc :		/* This is the PECOFF .reloc section! */
  {
    KEEP (*(.reloc))
  }
  . = ALIGN(4096);
  .dynsym   : { *(.dynsym) }
  . = ALIGN(4096);
  .dynstr   : { *(.dynstr) }
  . = DATA_SEGMENT_END (.);
  /DISCARD/ :
  {
    *(.rela.plabel)
    *(.rela.reloc)
    *(.IA_64.unwind*)
    *(.IA64.unwind*)
  }
}

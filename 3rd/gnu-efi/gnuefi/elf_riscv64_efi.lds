/* SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS {
.text 0x0 :
	{
		_text = .;
		*(.text.head)
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
		*(.srodata)
		*(.rodata*)
		. = ALIGN(16);
	}
	_etext = .;
	_text_size = . - _text;
. = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
.dynamic  :
	{ *(.dynamic) }
.data :
	ALIGN(4096)
	{
		_data = .;
		*(.sdata)
		*(.data)
		*(.data1)
		*(.data.*)
		*(.got.plt)
		*(.got)

		/* the EFI loader doesn't seem to like a .bss section, so we stick
		   it all into .data: */
		. = ALIGN(16);
		_bss = .;
		*(.sbss)
		*(.scommon)
		*(.dynbss)
		*(.bss*)
		*(COMMON)
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
.dynsym   :
	{ *(.dynsym) }
	. = ALIGN(4096);
.dynstr   :
	{ *(.dynstr) }
	. = ALIGN(4096);
.note.gnu.build-id :
	{ *(.note.gnu.build-id) }
. = DATA_SEGMENT_END (.);
/DISCARD/ :
	{
		*(.rel.reloc)
		*(.eh_frame)
		*(.note.GNU-stack)
	}
.comment 0 :
	{ *(.comment) }
}

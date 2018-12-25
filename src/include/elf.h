
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h
// elf.h for MRNIU/SimpleKernel.


#ifndef _ELF_H_
#define _ELF_H_

#include "stdint.h"

typedef
  struct dynamic {
		int32_t tag;
		union {
				int32_t val;
				uint32_t ptr;
		} un;
} Elf32_Dyn;

typedef
  struct elf32_rel {
		uint32_t offset;
		uint32_t info;
} Elf32_Rel;

typedef
  struct elf32_rela {
		uint32_t offset;
		uint32_t info;
		int32_t addend;
} Elf32_Rela;

// ELF 格式符号
typedef
  struct elf32_symbol {
		uint32_t name;
		uint32_t value;
		uint32_t size;
		unsigned char info;
		unsigned char other;
		uint16_t shndx;
} elf_symbol_t;


// ELF 文件头
#define EI_NIDENT 16
typedef
  struct elf32_hdr {
		unsigned char ident[EI_NIDENT];
		uint16_t type;
		uint16_t machine;
		uint32_t version;
		uint32_t entry;  /* Entry point */
		uint32_t phoff;
		uint32_t shoff;
		uint32_t flags;
		uint16_t ehsize;
		uint16_t phentsize;
		uint16_t phnum;
		uint16_t shentsize;
		uint16_t shnum;
		uint16_t shstrndx;
} Elf32_Ehdr;


typedef
  struct elf32_phdr {
		uint32_t type;
		uint32_t offset;
		uint32_t vaddr;
		uint32_t paddr;
		uint32_t filesz;
		uint32_t memsz;
		uint32_t flags;
		uint32_t align;
} Elf32_Phdr;

// ELF 格式头
typedef
  struct elf32_section_header {
		uint32_t name;
		uint32_t type;
		uint32_t flags;
		uint32_t addr;
		uint32_t offset;
		uint32_t size;
		uint32_t link;
		uint32_t info;
		uint32_t addralign;
		uint32_t entsize;
} elf32_section_header_t;

#define EI_MAG0  0  /* e_ident[] indexes */
#define EI_MAG1  1
#define EI_MAG2  2
#define EI_MAG3  3
#define EI_CLASS 4
#define EI_DATA  5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_PAD  8

#define ELFMAG0  0x7f  /* EI_MAG */
#define ELFMAG1  'E'
#define ELFMAG2  'L'
#define ELFMAG3  'F'
#define ELFMAG  "\177ELF"
#define SELFMAG  4

/* Note header in a PT_NOTE section */
typedef
  struct elf32_note {
		uint32_t namesz; /* Name size */
		uint32_t descsz; /* Content size */
		uint32_t type;  /* Content type */
} Elf32_Nhdr;



#endif

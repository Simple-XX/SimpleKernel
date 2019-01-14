
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-5.html
// elf.h for MRNIU/SimpleKernel.

#ifndef _ELF_H_
#define _ELF_H_

#include "stdint.h"
#include "multiboot2.h"

#define ELF32_ST_TYPE(i) ((i)&0xf)

// ELF 格式区段头
typedef
  struct elf_section_header_t {
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
} __attribute__((packed)) elf_section_header_t;

// ELF 格式符号
typedef
  struct elf_symbol_t {
		uint32_t name;
		uint32_t value;
		uint32_t size;
		uint8_t info;
		uint8_t other;
		uint16_t shndx;
} __attribute__((packed)) elf_symbol_t;

// ELF 信息
typedef
  struct elf_t {
		elf_symbol_t *symtab;
		uint32_t symtabsz;
		const char   *strtab;
		uint32_t strtabsz;
} elf_t;

#endif

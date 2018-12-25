
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h
// elf.h for MRNIU/SimpleKernel.


#ifndef _ELF_H_
#define _ELF_H_

#include "stdint.h"

#define EI_NIDENT 16
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
typedef
  struct elf32_hdr {
		unsigned char ident[EI_NIDENT]; // 见 EI_ 宏定义
		uint16_t type;  // ELF 文件类型
		uint16_t machine; // ELF 文件 CPU 平台属性
		uint32_t version; // ELF 版本号
		uint32_t entry; // 入口点
		uint32_t phoff; //
		uint32_t shoff; // 段表在文件中的偏移
		uint32_t flags; // 标志位 0x0
		uint16_t ehsize;  // ELF 文件头本身的大小
		uint16_t phentsize; //
		uint16_t phnum; //
		uint16_t shentsize;// 段表描述符的大小
		uint16_t shnum; // 段表描述符数量
		uint16_t shstrndx;  // 段表字符串所在的段在表中的下标
} Elf32_Ehdr;

// ELF 段描述符
typedef
  struct elf32_section_header {
		uint32_t name;  // 段名，位于 .shstrtab
		uint32_t type;  // 段类型
		uint32_t flags; // 段标志
		uint32_t addr;  // 段虚拟地址
		uint32_t offset;  // 段偏移
		uint32_t size;  // 段长度
		uint32_t link;  //
		uint32_t info;  // 段链接信息
		uint32_t addralign; // 端地址对齐
		uint32_t entsize; // 项长度
} elf32_section_header_t;


// ELF 信息
typedef
  struct elf_info {
		elf_symbol_t * symtab;
		uint32_t symtabsz;
		const char * strtab;
		uint32_t strtabsz;
} elf_info_t;


#endif

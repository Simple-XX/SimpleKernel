
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h
// elf.h for MRNIU/SimpleKernel.


#ifndef _ELF_H_
#define _ELF_H_

#include "stdint.h"

typedef uint16_t Elf32_Half; // Unsigned half int
typedef uint32_t Elf32_Off; // Unsigned offset
typedef uint32_t Elf32_Addr; // Unsigned address
typedef uint32_t Elf32_Word; // Unsigned int
typedef int32_t Elf32_Sword;  // Signed int

# define ELF_RELOC_ERR -1

# define ELF_NIDENT 16
typedef struct {
	uint8_t e_ident[ELF_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

// enum Elf_Ident {
// 	EI_MAG0  = 0,// 0x7F
// 	EI_MAG1  = 1,// 'E'
// 	EI_MAG2  = 2,// 'L'
// 	EI_MAG3  = 3,// 'F'
// 	EI_CLASS = 4, // Architecture (32/64)
// 	EI_DATA  = 5,// Byte Order
// 	EI_VERSION = 6, // ELF Version
// 	EI_OSABI = 7, // OS Specific
// 	EI_ABIVERSION = 8, // OS Specific
// 	EI_PAD  = 9// Padding
// };

# define ELFMAG0 0x7F // e_ident[EI_MAG0]
# define ELFMAG1 'E'  // e_ident[EI_MAG1]
# define ELFMAG2 'L'  // e_ident[EI_MAG2]
# define ELFMAG3 'F'  // e_ident[EI_MAG3]

# define ELFDATA2LSB ( 1 )  // Little Endian
# define ELFCLASS32 ( 1 )  // 32-bit Architecture

// enum Elf_Type {
// 	ET_NONE  = 0,// Unkown Type
// 	ET_REL  = 1,// Relocatable File
// 	ET_EXEC  = 2// Executable File
// };

# define EM_386  ( 3 )  // x86 Machine Type
# define EV_CURRENT ( 1 )  // ELF Current Version

typedef struct {
	Elf32_Word sh_name; // Section name, index in string tbl
	Elf32_Word sh_type; // Type of section
	Elf32_Word sh_flags; // Miscellaneous section attributes
	Elf32_Addr sh_addr; // Section virtual addr at execution
	Elf32_Off sh_offset; // Section file offset
	Elf32_Word sh_size; // Size of section in bytes
	Elf32_Word sh_link; // Index of another section
	Elf32_Word sh_info; // Additional section information
	Elf32_Word sh_addralign; // Section alignment
	Elf32_Word sh_entsize; // Entry size if section holds table
} Elf32_Shdr;

# define SHN_UNDEF ( 0x00 ) // Undefined/Not present

// enum ShT_Types {
// 	SHT_NULL = 0, // Null section
// 	SHT_PROGBITS = 1, // Program information
// 	SHT_SYMTAB = 2, // Symbol table
// 	SHT_STRTAB = 3, // String table
// 	SHT_RELA = 4, // Relocation (w/ addend)
// 	SHT_NOBITS = 8, // Not present in file
// 	SHT_REL  = 9,// Relocation (no addend)
// };
//
// enum ShT_Attributes {
// 	SHF_WRITE = 0x01, // Writable section
// 	SHF_ALLOC = 0x02 // Exists in memory
// };

typedef struct {
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	uint8_t st_info;
	uint8_t st_other;
	Elf32_Half st_shndx;
} Elf32_Sym;

# define ELF32_ST_BIND(INFO) ( ( INFO ) >> 4 )
# define ELF32_ST_TYPE(INFO) ( ( INFO ) & 0x0F )

// enum StT_Bindings {
// 	STB_LOCAL  = 0,// Local scope
// 	STB_GLOBAL  = 1,// Global scope
// 	STB_WEAK  = 2// Weak, (ie. __attribute__((weak)))
// };
//
// enum StT_Types {
// 	STT_NOTYPE  = 0,// No type
// 	STT_OBJECT  = 1,// Variables, arrays, etc.
// 	STT_FUNC  = 2// Methods or functions
// };

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} Elf32_Rel;

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
	Elf32_Sword r_addend;
} Elf32_Rela;


# define ELF32_R_SYM(INFO) ( ( INFO ) >> 8 )
# define ELF32_R_TYPE(INFO) ( (uint8_t)( INFO ) )

// enum RtT_Types {
// 	R_386_NONE  = 0,// No relocation
// 	R_386_32  = 1,// Symbol + Offset
// 	R_386_PC32  = 2// Symbol + Offset - Section Offset
// };

// Program header
typedef struct {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

// ELF 信息
typedef
        struct elf_t {
	Elf32_Sym *symtab;
	uint32_t symtabsz;
	const char *strtab;
	uint32_t strtabsz;
} elf_t;

static elf_t kernel_elf __attribute__( ( unused ) );
// bool elf_check_file(struct multiboot_tag_elf_sections * tag);



#endif

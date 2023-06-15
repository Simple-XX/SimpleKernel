/*
 * utils/efirom.c
 *
 * Copyright (C) 2009 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * modifications for gnuefi by bzt (bztsrc@gitlab)
 *
 * This file is part of the POSIX-UEFI package.
 * @brief small tool to convert an .efi file to a .rom file
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>

#define eprintf(...) fprintf ( stderr, __VA_ARGS__ )

/**
 * Print help
 *
 * @v program_name	Program name
 */
static void print_help ( const char *program_name ) {
	eprintf ( "POSIX-UEFI utils - efirom by Michael Brown GPL\r\n\r\n" );
	eprintf ( "%s [--vendor=VVVV] [--device=DDDD] "
		  "infile outfile\n", program_name );
}

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((UINT64)(EFI_SIGNATURE_32(E,F,G,H)) << 32))
typedef uint64_t    UINTN;
typedef UINTN       EFI_TPL;

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
        uint8_t                           Type;
        uint8_t                           SubType;
        uint8_t                           Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct _EFI_DEVICE_PATH_PROTOCOL _EFI_DEVICE_PATH;
typedef EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH;

#ifndef _EFI_LINK_H
#define _EFI_LINK_H

/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    link.h (renamed efilink.h to avoid conflicts)

Abstract:

    EFI link list macro's



Revision History

--*/

#ifndef EFI_NT_EMUL

//
// List entry - doubly linked list
//

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY  *Flink;
    struct _LIST_ENTRY  *Blink;
} LIST_ENTRY;

#endif


//
//  VOID
//  InitializeListHead(
//      LIST_ENTRY *ListHead
//      );
//

#define InitializeListHead(ListHead) \
    (ListHead)->Flink = ListHead;    \
    (ListHead)->Blink = ListHead;

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define _RemoveEntryList(Entry) {       \
        LIST_ENTRY *_Blink, *_Flink;    \
        _Flink = (Entry)->Flink;        \
        _Blink = (Entry)->Blink;        \
        _Blink->Flink = _Flink;         \
        _Flink->Blink = _Blink;         \
        }

#if EFI_DEBUG
    #define RemoveEntryList(Entry)                      \
        _RemoveEntryList(Entry);                        \
        (Entry)->Flink = (LIST_ENTRY *) BAD_POINTER;    \
        (Entry)->Blink = (LIST_ENTRY *) BAD_POINTER;
#else
    #define RemoveEntryList(Entry)      \
        _RemoveEntryList(Entry);
#endif

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    LIST_ENTRY *_ListHead, *_Blink;     \
    _ListHead = (ListHead);             \
    _Blink = _ListHead->Blink;          \
    (Entry)->Flink = _ListHead;         \
    (Entry)->Blink = _Blink;            \
    _Blink->Flink = (Entry);            \
    _ListHead->Blink = (Entry);         \
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    LIST_ENTRY *_ListHead, *_Flink;     \
    _ListHead = (ListHead);             \
    _Flink = _ListHead->Flink;          \
    (Entry)->Flink = _Flink;            \
    (Entry)->Blink = _ListHead;         \
    _Flink->Blink = (Entry);            \
    _ListHead->Flink = (Entry);         \
    }

//  VOID
//  SwapListEntries(
//      PLIST_ENTRY Entry1,
//      PLIST_ENTRY Entry2
//      );
//
// Put Entry2 before Entry1
//
#define SwapListEntries(Entry1,Entry2) {\
    LIST_ENTRY *Entry1Flink, *Entry1Blink;     \
    LIST_ENTRY *Entry2Flink, *Entry2Blink;     \
    Entry2Flink = (Entry2)->Flink;             \
    Entry2Blink = (Entry2)->Blink;             \
    Entry1Flink = (Entry1)->Flink;             \
    Entry1Blink = (Entry1)->Blink;             \
    Entry2Blink->Flink = Entry2Flink;       \
    Entry2Flink->Blink = Entry2Blink;        \
    (Entry2)->Flink = Entry1;               \
    (Entry2)->Blink = Entry1Blink;          \
    Entry1Blink->Flink = (Entry2);            \
    (Entry1)->Blink = (Entry2);             \
    }

//
//  EFI_FIELD_OFFSET - returns the byte offset to a field within a structure
//

#define EFI_FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))

//
//  CONTAINING_RECORD - returns a pointer to the structure
//      from one of it's elements.
//

#define _CR(Record, TYPE, Field)  \
    ((TYPE *) ( (CHAR8 *)(Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#if EFI_DEBUG
    #define CR(Record, TYPE, Field, Sig)     \
        _CR(Record, TYPE, Field)->Signature != Sig ?        \
            (TYPE *) ASSERT_STRUCT(_CR(Record, TYPE, Field), Record) : \
            _CR(Record, TYPE, Field)
#else
    #define CR(Record, TYPE, Field, Signature)   \
        _CR(Record, TYPE, Field)
#endif


//
// A lock structure
//

typedef struct _FLOCK {
    EFI_TPL     Tpl;
    EFI_TPL     OwnerTpl;
    UINTN       Lock;
} FLOCK;

#endif

/*
    PE32+ header file
 */
#ifndef _PE_H
#define _PE_H

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00
#define IMAGE_EDOS_SIGNATURE                0x44454550  // PEED


typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    uint16_t   e_magic;                     // Magic number
    uint16_t   e_cblp;                      // Bytes on last page of file
    uint16_t   e_cp;                        // Pages in file
    uint16_t   e_crlc;                      // Relocations
    uint16_t   e_cparhdr;                   // Size of header in paragraphs
    uint16_t   e_minalloc;                  // Minimum extra paragraphs needed
    uint16_t   e_maxalloc;                  // Maximum extra paragraphs needed
    uint16_t   e_ss;                        // Initial (relative) SS value
    uint16_t   e_sp;                        // Initial SP value
    uint16_t   e_csum;                      // Checksum
    uint16_t   e_ip;                        // Initial IP value
    uint16_t   e_cs;                        // Initial (relative) CS value
    uint16_t   e_lfarlc;                    // File address of relocation table
    uint16_t   e_ovno;                      // Overlay number
    uint16_t   e_res[4];                    // Reserved words
    uint16_t   e_oemid;                     // OEM identifier (for e_oeminfo)
    uint16_t   e_oeminfo;                   // OEM information; e_oemid specific
    uint16_t   e_res2[10];                  // Reserved words
    uint32_t   e_lfanew;                    // File address of new exe header
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {      // OS/2 .EXE header
    uint16_t   ne_magic;                    // Magic number
    uint8_t    ne_ver;                      // Version number
    uint8_t    ne_rev;                      // Revision number
    uint16_t   ne_enttab;                   // Offset of Entry Table
    uint16_t   ne_cbenttab;                 // Number of bytes in Entry Table
    uint32_t   ne_crc;                      // Checksum of whole file
    uint16_t   ne_flags;                    // Flag uint16_t
    uint16_t   ne_autodata;                 // Automatic data segment number
    uint16_t   ne_heap;                     // Initial heap allocation
    uint16_t   ne_stack;                    // Initial stack allocation
    uint32_t   ne_csip;                     // Initial CS:IP setting
    uint32_t   ne_sssp;                     // Initial SS:SP setting
    uint16_t   ne_cseg;                     // Count of file segments
    uint16_t   ne_cmod;                     // Entries in Module Reference Table
    uint16_t   ne_cbnrestab;                // Size of non-resident name table
    uint16_t   ne_segtab;                   // Offset of Segment Table
    uint16_t   ne_rsrctab;                  // Offset of Resource Table
    uint16_t   ne_restab;                   // Offset of resident name table
    uint16_t   ne_modtab;                   // Offset of Module Reference Table
    uint16_t   ne_imptab;                   // Offset of Imported Names Table
    uint32_t   ne_nrestab;                  // Offset of Non-resident Names Table
    uint16_t   ne_cmovent;                  // Count of movable entries
    uint16_t   ne_align;                    // Segment alignment shift count
    uint16_t   ne_cres;                     // Count of resource segments
    uint8_t    ne_exetyp;                   // Target Operating system
    uint8_t    ne_flagsothers;              // Other .EXE flags
    uint16_t   ne_pretthunks;               // offset to return thunks
    uint16_t   ne_psegrefbytes;             // offset to segment ref. bytes
    uint16_t   ne_swaparea;                 // Minimum code swap area size
    uint16_t   ne_expver;                   // Expected Windows version number
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

//
// File header format.
//

typedef struct _IMAGE_FILE_HEADER {
    uint16_t   Machine;
    uint16_t   NumberOfSections;
    uint32_t   TimeDateStamp;
    uint32_t   PointerToSymbolTable;
    uint32_t   NumberOfSymbols;
    uint16_t   SizeOfOptionalHeader;
    uint16_t   Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20

#define IMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002  // File is executable  (i.e. no unresolved externel references).
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in .DBG file
#define IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define IMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I386              0x14c   // Intel 386.
#define IMAGE_FILE_MACHINE_R3000             0x162   // MIPS little-endian, 0540 big-endian
#define IMAGE_FILE_MACHINE_R4000             0x166   // MIPS little-endian
#define IMAGE_FILE_MACHINE_ALPHA             0x184   // Alpha_AXP
#define IMAGE_FILE_MACHINE_ARMTHUMB_MIXED    0x1c2   // Arm/Thumb
#define IMAGE_FILE_MACHINE_POWERPC           0x1F0   // IBM PowerPC Little-Endian
#define IMAGE_FILE_MACHINE_IA64              0x200   // IA-64
#define IMAGE_FILE_MACHINE_TAHOE             0x7cc   // Intel EM machine
#define IMAGE_FILE_MACHINE_EBC               0xebc   // EFI Byte Code
#define IMAGE_FILE_MACHINE_X64               0x8664  // x86_64
//
// Directory format.
//

typedef struct _IMAGE_DATA_DIRECTORY {
    uint32_t   VirtualAddress;
    uint32_t   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

//
// Optional header format.
//

typedef struct _IMAGE_OPTIONAL_HEADER {
    //
    // Standard fields.
    //

    uint16_t    Magic;
    uint8_t     MajorLinkerVersion;
    uint8_t     MinorLinkerVersion;
    uint32_t    SizeOfCode;
    uint32_t    SizeOfInitializedData;
    uint32_t    SizeOfUninitializedData;
    uint32_t    AddressOfEntryPoint;
    uint32_t    BaseOfCode;
    uint32_t    BaseOfData;

    //
    // NT additional fields.
    //

    uint32_t   ImageBase;
    uint32_t   SectionAlignment;
    uint32_t   FileAlignment;
    uint16_t   MajorOperatingSystemVersion;
    uint16_t   MinorOperatingSystemVersion;
    uint16_t   MajorImageVersion;
    uint16_t   MinorImageVersion;
    uint16_t   MajorSubsystemVersion;
    uint16_t   MinorSubsystemVersion;
    uint32_t   Reserved1;
    uint32_t   SizeOfImage;
    uint32_t   SizeOfHeaders;
    uint32_t   CheckSum;
    uint16_t   Subsystem;
    uint16_t   DllCharacteristics;
    uint32_t   SizeOfStackReserve;
    uint32_t   SizeOfStackCommit;
    uint32_t   SizeOfHeapReserve;
    uint32_t   SizeOfHeapCommit;
    uint32_t   LoaderFlags;
    uint32_t   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
    uint16_t  Magic;
    uint8_t   MajorLinkerVersion;
    uint8_t   MinorLinkerVersion;
    uint32_t  SizeOfCode;
    uint32_t  SizeOfInitializedData;
    uint32_t  SizeOfUninitializedData;
    uint32_t  AddressOfEntryPoint;
    uint32_t  BaseOfCode;
    uint32_t  BaseOfData;
    uint32_t  BaseOfBss;
    uint32_t  GprMask;
    uint32_t  CprMask[4];
    uint32_t  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER      224

#define IMAGE_NT_OPTIONAL_HDR_MAGIC        0x10b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

typedef struct _IMAGE_NT_HEADERS {
    uint32_t Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((uint32_t)ntheader +                                                  \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))


// Subsystem Values

#define IMAGE_SUBSYSTEM_UNKNOWN              0   // Unknown subsystem.
#define IMAGE_SUBSYSTEM_NATIVE               1   // Image doesn't require a subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2   // Image runs in the Windows GUI subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3   // Image runs in the Windows character subsystem.
#define IMAGE_SUBSYSTEM_OS2_CUI              5   // image runs in the OS/2 character subsystem.
#define IMAGE_SUBSYSTEM_POSIX_CUI            7   // image run  in the Posix character subsystem.


// Directory Entries

#define IMAGE_DIRECTORY_ENTRY_EXPORT         0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6   // Debug Directory
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7   // Description String
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8   // Machine Value (MIPS GP)
#define IMAGE_DIRECTORY_ENTRY_TLS            9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10   // Load Configuration Directory

//
// Section header format.
//

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    uint8_t   Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            uint32_t   PhysicalAddress;
            uint32_t   VirtualSize;
    } Misc;
    uint32_t   VirtualAddress;
    uint32_t   SizeOfRawData;
    uint32_t   PointerToRawData;
    uint32_t   PointerToRelocations;
    uint32_t   PointerToLinenumbers;
    uint16_t   NumberOfRelocations;
    uint16_t   NumberOfLinenumbers;
    uint32_t   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

#define IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
#define IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000  //

#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

//
// Symbol format.
//


#define IMAGE_SIZEOF_SYMBOL                  18

//
// Section values.
//
// Symbols have a section number of the section in which they are
// defined. Otherwise, section numbers have the following meanings:
//

#define IMAGE_SYM_UNDEFINED           (uint16_t)0           // Symbol is undefined or is common.
#define IMAGE_SYM_ABSOLUTE            (uint16_t)-1          // Symbol is an absolute value.
#define IMAGE_SYM_DEBUG               (uint16_t)-2          // Symbol is a special debug item.

//
// Type (fundamental) values.
//

#define IMAGE_SYM_TYPE_NULL                  0           // no type.
#define IMAGE_SYM_TYPE_VOID                  1           //
#define IMAGE_SYM_TYPE_CHAR                  2           // type character.
#define IMAGE_SYM_TYPE_SHORT                 3           // type short integer.
#define IMAGE_SYM_TYPE_INT                   4           //
#define IMAGE_SYM_TYPE_LONG                  5           //
#define IMAGE_SYM_TYPE_FLOAT                 6           //
#define IMAGE_SYM_TYPE_DOUBLE                7           //
#define IMAGE_SYM_TYPE_STRUCT                8           //
#define IMAGE_SYM_TYPE_UNION                 9           //
#define IMAGE_SYM_TYPE_ENUM                  10          // enumeration.
#define IMAGE_SYM_TYPE_MOE                   11          // member of enumeration.
#define IMAGE_SYM_TYPE_BYTE                  12          //
#define IMAGE_SYM_TYPE_WORD                  13          //
#define IMAGE_SYM_TYPE_UINT                  14          //
#define IMAGE_SYM_TYPE_DWORD                 15          //

//
// Type (derived) values.
//

#define IMAGE_SYM_DTYPE_NULL                 0           // no derived type.
#define IMAGE_SYM_DTYPE_POINTER              1           // pointer.
#define IMAGE_SYM_DTYPE_FUNCTION             2           // function.
#define IMAGE_SYM_DTYPE_ARRAY                3           // array.

//
// Storage classes.
//

#define IMAGE_SYM_CLASS_END_OF_FUNCTION      (BYTE )-1
#define IMAGE_SYM_CLASS_NULL                 0
#define IMAGE_SYM_CLASS_AUTOMATIC            1
#define IMAGE_SYM_CLASS_EXTERNAL             2
#define IMAGE_SYM_CLASS_STATIC               3
#define IMAGE_SYM_CLASS_REGISTER             4
#define IMAGE_SYM_CLASS_EXTERNAL_DEF         5
#define IMAGE_SYM_CLASS_LABEL                6
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL      7
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT     8
#define IMAGE_SYM_CLASS_ARGUMENT             9
#define IMAGE_SYM_CLASS_STRUCT_TAG           10
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION      11
#define IMAGE_SYM_CLASS_UNION_TAG            12
#define IMAGE_SYM_CLASS_TYPE_DEFINITION      13
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC     14
#define IMAGE_SYM_CLASS_ENUM_TAG             15
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM       16
#define IMAGE_SYM_CLASS_REGISTER_PARAM       17
#define IMAGE_SYM_CLASS_BIT_FIELD            18
#define IMAGE_SYM_CLASS_BLOCK                100
#define IMAGE_SYM_CLASS_FUNCTION             101
#define IMAGE_SYM_CLASS_END_OF_STRUCT        102
#define IMAGE_SYM_CLASS_FILE                 103
// new
#define IMAGE_SYM_CLASS_SECTION              104
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL        105

// type packing constants

#define N_BTMASK                            017
#define N_TMASK                             060
#define N_TMASK1                            0300
#define N_TMASK2                            0360
#define N_BTSHFT                            4
#define N_TSHIFT                            2

// MACROS

//
// Communal selection types.
//

#define IMAGE_COMDAT_SELECT_NODUPLICATES   1
#define IMAGE_COMDAT_SELECT_ANY            2
#define IMAGE_COMDAT_SELECT_SAME_SIZE      3
#define IMAGE_COMDAT_SELECT_EXACT_MATCH    4
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE    5

#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY 1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY   2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS     3


//
// Relocation format.
//

typedef struct _IMAGE_RELOCATION {
    uint32_t   VirtualAddress;
    uint32_t   SymbolTableIndex;
    uint16_t    Type;
} IMAGE_RELOCATION;

#define IMAGE_SIZEOF_RELOCATION              10

//
// I386 relocation types.
//

#define IMAGE_REL_I386_ABSOLUTE              0           // Reference is absolute, no relocation is necessary
#define IMAGE_REL_I386_DIR16                 01          // Direct 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_REL16                 02          // PC-relative 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32                 06          // Direct 32-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32NB               07          // Direct 32-bit reference to the symbols virtual address, base not included
#define IMAGE_REL_I386_SEG12                 011         // Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
#define IMAGE_REL_I386_SECTION               012
#define IMAGE_REL_I386_SECREL                013
#define IMAGE_REL_I386_REL32                 024         // PC-relative 32-bit reference to the symbols virtual address

//
// MIPS relocation types.
//

#define IMAGE_REL_MIPS_ABSOLUTE              0           // Reference is absolute, no relocation is necessary
#define IMAGE_REL_MIPS_REFHALF               01
#define IMAGE_REL_MIPS_REFWORD               02
#define IMAGE_REL_MIPS_JMPADDR               03
#define IMAGE_REL_MIPS_REFHI                 04
#define IMAGE_REL_MIPS_REFLO                 05
#define IMAGE_REL_MIPS_GPREL                 06
#define IMAGE_REL_MIPS_LITERAL               07
#define IMAGE_REL_MIPS_SECTION               012
#define IMAGE_REL_MIPS_SECREL                013
#define IMAGE_REL_MIPS_REFWORDNB             042
#define IMAGE_REL_MIPS_PAIR                  045

//
// Alpha Relocation types.
//

#define IMAGE_REL_ALPHA_ABSOLUTE             0x0
#define IMAGE_REL_ALPHA_REFLONG              0x1
#define IMAGE_REL_ALPHA_REFQUAD              0x2
#define IMAGE_REL_ALPHA_GPREL32              0x3
#define IMAGE_REL_ALPHA_LITERAL              0x4
#define IMAGE_REL_ALPHA_LITUSE               0x5
#define IMAGE_REL_ALPHA_GPDISP               0x6
#define IMAGE_REL_ALPHA_BRADDR               0x7
#define IMAGE_REL_ALPHA_HINT                 0x8
#define IMAGE_REL_ALPHA_INLINE_REFLONG       0x9
#define IMAGE_REL_ALPHA_REFHI                0xA
#define IMAGE_REL_ALPHA_REFLO                0xB
#define IMAGE_REL_ALPHA_PAIR                 0xC
#define IMAGE_REL_ALPHA_MATCH                0xD
#define IMAGE_REL_ALPHA_SECTION              0xE
#define IMAGE_REL_ALPHA_SECREL               0xF
#define IMAGE_REL_ALPHA_REFLONGNB            0x10

//
// IBM PowerPC relocation types.
//

#define IMAGE_REL_PPC_ABSOLUTE 0x0000  // NOP
#define IMAGE_REL_PPC_ADDR64   0x0001  // 64-bit address
#define IMAGE_REL_PPC_ADDR32   0x0002  // 32-bit address
#define IMAGE_REL_PPC_ADDR24   0x0003  // 26-bit address, shifted left 2 (branch absolute)
#define IMAGE_REL_PPC_ADDR16   0x0004  // 16-bit address
#define IMAGE_REL_PPC_ADDR14   0x0005  // 16-bit address, shifted left 2 (load doubleword)
#define IMAGE_REL_PPC_REL24    0x0006  // 26-bit PC-relative offset, shifted left 2 (branch relative)
#define IMAGE_REL_PPC_REL14    0x0007  // 16-bit PC-relative offset, shifted left 2 (br cond relative)
#define IMAGE_REL_PPC_TOCREL16 0x0008  // 16-bit offset from TOC base
#define IMAGE_REL_PPC_TOCREL14 0x0009  // 16-bit offset from TOC base, shifted left 2 (load doubleword)

#define IMAGE_REL_PPC_ADDR32NB 0x000A  // 32-bit addr w/o image base
#define IMAGE_REL_PPC_SECREL   0x000B  // va of containing section (as in an image sectionhdr)
#define IMAGE_REL_PPC_SECTION  0x000C  // sectionheader number
#define IMAGE_REL_PPC_IFGLUE   0x000D  // substitute TOC restore instruction iff symbol is glue code
#define IMAGE_REL_PPC_IMGLUE   0x000E  // symbol is glue code; virtual address is TOC restore instruction

#define IMAGE_REL_PPC_TYPEMASK 0x00FF  // mask to isolate above values in IMAGE_RELOCATION.Type

// Flag bits in IMAGE_RELOCATION.TYPE

#define IMAGE_REL_PPC_NEG      0x0100  // subtract reloc value rather than adding it
#define IMAGE_REL_PPC_BRTAKEN  0x0200  // fix branch prediction bit to predict branch taken
#define IMAGE_REL_PPC_BRNTAKEN 0x0400  // fix branch prediction bit to predict branch not taken
#define IMAGE_REL_PPC_TOCDEFN  0x0800  // toc slot defined in file (or, data in toc)

//
// Based relocation format.
//

typedef struct _IMAGE_BASE_RELOCATION {
    uint32_t   VirtualAddress;
    uint32_t   SizeOfBlock;
//  uint16_t    TypeOffset[1];
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

//
// Based relocation types.
//

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_DIR64                 10

//
// Line number format.
//

typedef struct _IMAGE_LINENUMBER {
    union {
        uint32_t   SymbolTableIndex;               // Symbol table index of function name if Linenumber is 0.
        uint32_t   VirtualAddress;                 // Virtual address of line number.
    } Type;
    uint16_t    Linenumber;                         // Line number.
} IMAGE_LINENUMBER;

#define IMAGE_SIZEOF_LINENUMBER              6

//
// Archive format.
//

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       "//              "

typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    uint8_t     Name[16];                          // File member name - `/' terminated.
    uint8_t     Date[12];                          // File member date - decimal.
    uint8_t     UserID[6];                         // File member user id - decimal.
    uint8_t     GroupID[6];                        // File member group id - decimal.
    uint8_t     Mode[8];                           // File member mode - octal.
    uint8_t     Size[10];                          // File member size - decimal.
    uint8_t     EndHeader[2];                      // String to end header.
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

//
// DLL support.
//

//
// Export Format
//

typedef struct _IMAGE_EXPORT_DIRECTORY {
    uint32_t   Characteristics;
    uint32_t   TimeDateStamp;
    uint16_t   MajorVersion;
    uint16_t   MinorVersion;
    uint32_t   Name;
    uint32_t   Base;
    uint32_t   NumberOfFunctions;
    uint32_t   NumberOfNames;
    uint32_t   *AddressOfFunctions;
    uint32_t   *AddressOfNames;
    uint32_t   *AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

//
// Import Format
//

typedef struct _IMAGE_IMPORT_BY_NAME {
    uint16_t    Hint;
    uint8_t     Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA {
    union {
        uint32_t Function;
        uint32_t Ordinal;
        PIMAGE_IMPORT_BY_NAME AddressOfData;
    } u1;
} IMAGE_THUNK_DATA, *PIMAGE_THUNK_DATA;

#define IMAGE_ORDINAL_FLAG 0x80000000
#define IMAGE_SNAP_BY_ORDINAL(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG) != 0)
#define IMAGE_ORDINAL(Ordinal) (Ordinal & 0xffff)

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    uint32_t   Characteristics;
    uint32_t   TimeDateStamp;
    uint32_t   ForwarderChain;
    uint32_t   Name;
    PIMAGE_THUNK_DATA FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;

#endif

#ifndef _PCI22_H
#define _PCI22_H

/*++

Copyright (c) 1999  Intel Corporation

Module Name:

    pci22.h

Abstract:
    Support for PCI 2.2 standard.




Revision History

--*/

#ifdef SOFT_SDV
#define PCI_MAX_BUS     1
#else
#define PCI_MAX_BUS     255
#endif

#define PCI_MAX_DEVICE  31
#define PCI_MAX_FUNC    7

//
// Command
//
#define PCI_VGA_PALETTE_SNOOP_DISABLED   0x20

#pragma pack(1)
typedef struct {
    uint16_t      VendorId;
    uint16_t      DeviceId;
    uint16_t      Command;
    uint16_t      Status;
    uint8_t       RevisionID;
    uint8_t       ClassCode[3];
    uint8_t       CacheLineSize;
    uint8_t       LaytencyTimer;
    uint8_t       HeaderType;
    uint8_t       BIST;
} PCI_DEVICE_INDEPENDENT_REGION;

typedef struct {
    uint32_t      Bar[6];
    uint32_t      CISPtr;
    uint16_t      SubsystemVendorID;
    uint16_t      SubsystemID;
    uint32_t      ExpansionRomBar;
    uint32_t      Reserved[2];
    uint8_t       InterruptLine;
    uint8_t       InterruptPin;
    uint8_t       MinGnt;
    uint8_t       MaxLat;
} PCI_DEVICE_HEADER_TYPE_REGION;

typedef struct {
    PCI_DEVICE_INDEPENDENT_REGION   Hdr;
    PCI_DEVICE_HEADER_TYPE_REGION   Device;
} PCI_TYPE00;

typedef struct {
    uint32_t      Bar[2];
    uint8_t       PrimaryBus;
    uint8_t       SecondaryBus;
    uint8_t       SubordinateBus;
    uint8_t       SecondaryLatencyTimer;
    uint8_t       IoBase;
    uint8_t       IoLimit;
    uint16_t      SecondaryStatus;
    uint16_t      MemoryBase;
    uint16_t      MemoryLimit;
    uint16_t      PrefetchableMemoryBase;
    uint16_t      PrefetchableMemoryLimit;
    uint32_t      PrefetchableBaseUpper32;
    uint32_t      PrefetchableLimitUpper32;
    uint16_t      IoBaseUpper16;
    uint16_t      IoLimitUpper16;
    uint32_t      Reserved;
    uint32_t      ExpansionRomBAR;
    uint8_t       InterruptLine;
    uint8_t       InterruptPin;
    uint16_t      BridgeControl;
} PCI_BRIDGE_CONTROL_REGISTER;

#define PCI_CLASS_DISPLAY_CTRL          0x03
#define PCI_CLASS_VGA                   0x00

#define PCI_CLASS_BRIDGE                0x06
#define PCI_CLASS_ISA                   0x01
#define PCI_CLASS_ISA_POSITIVE_DECODE   0x80

#define PCI_CLASS_NETWORK               0x02
#define PCI_CLASS_ETHERNET              0x00

#define HEADER_TYPE_DEVICE              0x00
#define HEADER_TYPE_PCI_TO_PCI_BRIDGE   0x01
#define HEADER_TYPE_MULTI_FUNCTION      0x80
#define HEADER_LAYOUT_CODE              0x7f

#define IS_PCI_BRIDGE(_p) ((((_p)->Hdr.HeaderType) & HEADER_LAYOUT_CODE) == HEADER_TYPE_PCI_TO_PCI_BRIDGE)
#define IS_PCI_MULTI_FUNC(_p)   (((_p)->Hdr.HeaderType) & HEADER_TYPE_MULTI_FUNCTION)

typedef struct {
    PCI_DEVICE_INDEPENDENT_REGION   Hdr;
    PCI_BRIDGE_CONTROL_REGISTER     Bridge;
} PCI_TYPE01;

typedef struct {
    uint8_t   Register;
    uint8_t   Function;
    uint8_t   Device;
    uint8_t   Bus;
    uint8_t   Reserved[4];
} DEFIO_PCI_ADDR;

typedef struct {
    uint32_t  Reg     : 8;
    uint32_t  Func    : 3;
    uint32_t  Dev     : 5;
    uint32_t  Bus     : 8;
    uint32_t  Reserved: 7;
    uint32_t  Enable  : 1;
} PCI_CONFIG_ACCESS_CF8;

#pragma pack()

#define EFI_ROOT_BRIDGE_LIST    'eprb'
typedef struct {
    UINTN           Signature;

    uint16_t          BridgeNumber;
    uint16_t          PrimaryBus;
    uint16_t          SubordinateBus;

    EFI_DEVICE_PATH *DevicePath;

    LIST_ENTRY      Link;
} PCI_ROOT_BRIDGE_ENTRY;


#define PCI_EXPANSION_ROM_HEADER_SIGNATURE        0xaa55
#define EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE 0x0EF1
#define PCI_DATA_STRUCTURE_SIGNATURE              EFI_SIGNATURE_32('P','C','I','R')

#pragma pack(1)
typedef struct {
    uint16_t          Signature;              // 0xaa55
    uint8_t           Reserved[0x16];
    uint16_t          PcirOffset;
} PCI_EXPANSION_ROM_HEADER;


typedef struct {
    uint16_t          Signature;              // 0xaa55
    uint16_t          InitializationSize;
    uint16_t          EfiSignature;           // 0x0EF1
    uint16_t          EfiSubsystem;
    uint16_t          EfiMachineType;
    uint8_t           Reserved[0x0A];
    uint16_t          EfiImageHeaderOffset;
    uint16_t          PcirOffset;
} EFI_PCI_EXPANSION_ROM_HEADER;

typedef struct {
    uint32_t          Signature;              // "PCIR"
    uint16_t          VendorId;
    uint16_t          DeviceId;
    uint16_t          Reserved0;
    uint16_t          Length;
    uint8_t           Revision;
    uint8_t           ClassCode[3];
    uint16_t          ImageLength;
    uint16_t          CodeRevision;
    uint8_t           CodeType;
    uint8_t           Indicator;
    uint16_t          Reserved1;
} PCI_DATA_STRUCTURE;
#pragma pack()

#endif


/** Command-line options */
struct options {
	uint16_t vendor;
	uint16_t device;
};

/**
 * Allocate memory
 *
 * @v len		Length of memory to allocate
 * @ret ptr		Pointer to allocated memory
 */
static void * xmalloc ( size_t len ) {
	void *ptr;

	ptr = malloc ( len );
	if ( ! ptr ) {
		eprintf ( "Could not allocate %zd bytes\n", len );
		exit ( 1 );
	}

	return ptr;
}

/**
 * Get file size
 *
 * @v file		File
 * @v len		File size
 */
/*
static size_t file_size ( FILE *file ) {
	ssize_t len;

	return len;
}
*/
/**
 * Read information from PE headers
 *
 * @v pe		PE file
 * @ret machine		Machine type
 * @ret subsystem	EFI subsystem
 */
static void read_pe_info ( void *pe, uint16_t *machine,
			   uint16_t *subsystem ) {
	IMAGE_DOS_HEADER *dos;
	union {
		IMAGE_NT_HEADERS nt64;
	} *nt;

	/* Locate NT header */
	dos = pe;
	nt = ( pe + dos->e_lfanew );

	/* issue 4: TianoCore demands subsystem 10, so we must use EFI_APPLICATION
	 * in the PE header. Therefore we force EFI_ROM subsystem in this code here. */
	if(nt->nt64.OptionalHeader.Subsystem == 10)
		nt->nt64.OptionalHeader.Subsystem = 13;

	/* Parse out PE information */
	*machine = nt->nt64.FileHeader.Machine;
	*subsystem = nt->nt64.OptionalHeader.Subsystem;
}

/**
 * Convert EFI image to ROM image
 *
 * @v pe		EFI file
 * @v rom		ROM file
 */
static void make_efi_rom ( FILE *pe, FILE *rom, struct options *opts ) {
	struct {
		EFI_PCI_EXPANSION_ROM_HEADER rom;
		PCI_DATA_STRUCTURE pci __attribute__ (( aligned ( 4 ) ));
		uint8_t checksum;
	} *headers;
	struct stat pe_stat;
	size_t pe_size;
	size_t rom_size;
	void *buf;
	void *payload;
	unsigned int i;
	uint8_t checksum;

	/* Determine PE file size */
	if ( fstat ( fileno ( pe ), &pe_stat ) != 0 ) {
		eprintf ( "Could not stat PE file: %s\n",
			  strerror ( errno ) );
		exit ( 1 );
	}
	pe_size = pe_stat.st_size;

	/* Determine ROM file size */
	rom_size = ( ( pe_size + sizeof ( *headers ) + 511 ) & ~511 );

	/* Allocate ROM buffer and read in PE file */
	buf = xmalloc ( rom_size );
	memset ( buf, 0, rom_size );
	headers = buf;
	payload = ( buf + sizeof ( *headers ) );
	if ( fread ( payload, pe_size, 1, pe ) != 1 ) {
		eprintf ( "Could not read PE file: %s\n",
			  strerror ( errno ) );
		exit ( 1 );
	}

	/* Construct ROM header */
	headers->rom.Signature = PCI_EXPANSION_ROM_HEADER_SIGNATURE;
	headers->rom.InitializationSize = ( rom_size / 512 );
	headers->rom.EfiSignature = EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE;
	read_pe_info ( payload, &headers->rom.EfiMachineType,
		       &headers->rom.EfiSubsystem );
	headers->rom.EfiImageHeaderOffset = sizeof ( *headers );
	headers->rom.PcirOffset =
		offsetof ( typeof ( *headers ), pci );
	headers->pci.Signature = PCI_DATA_STRUCTURE_SIGNATURE;
	headers->pci.VendorId = opts->vendor ? opts->vendor : 0x8086;
	headers->pci.DeviceId = opts->device ? opts->device : 0x100E;
	headers->pci.Length = sizeof ( headers->pci );
	headers->pci.ClassCode[0] = PCI_CLASS_NETWORK;
	headers->pci.ImageLength = ( rom_size / 512 );
	headers->pci.CodeType = 0x03; /* No constant in EFI headers? */
	headers->pci.Indicator = 0x80; /* No constant in EFI headers? */

	/* Fix image checksum */
	for ( i = 0, checksum = 0 ; i < rom_size ; i++ )
		checksum += *( ( uint8_t * ) buf + i );
	headers->checksum -= checksum;

	/* Write out ROM */
	if ( fwrite ( buf, rom_size, 1, rom ) != 1 ) {
		eprintf ( "Could not write ROM file: %s\n",
			  strerror ( errno ) );
		exit ( 1 );
	}
}

/**
 * Parse command-line options
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @v opts		Options structure to populate
 */
static int parse_options ( const int argc, char **argv,
			   struct options *opts ) {
	char *end;
	int c;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "vendor", required_argument, NULL, 'v' },
			{ "device", required_argument, NULL, 'd' },
			{ "help", 0, NULL, 'h' },
			{ 0, 0, 0, 0 }
		};

		if ( ( c = getopt_long ( argc, argv, "v:d:h",
					 long_options,
					 &option_index ) ) == -1 ) {
			break;
		}

		switch ( c ) {
		case 'v':
			opts->vendor = strtoul ( optarg, &end, 16 );
			if ( *end ) {
				eprintf ( "Invalid vendor \"%s\"\n", optarg );
				exit ( 2 );
			}
			break;
		case 'd':
			opts->device = strtoul ( optarg, &end, 16 );
			if ( *end ) {
				eprintf ( "Invalid device \"%s\"\n", optarg );
				exit ( 2 );
			}
			break;
		case 'h':
			print_help ( argv[0] );
			exit ( 0 );
		case '?':
		default:
			exit ( 2 );
		}
	}
	return optind;
}

int main ( int argc, char **argv ) {
	struct options opts = {
	};
	unsigned int infile_index;
	const char *infile_name;
	const char *outfile_name;
	FILE *infile;
	FILE *outfile;

	/* Parse command-line arguments */
	infile_index = parse_options ( argc, argv, &opts );
	if ( argc != ( infile_index + 2 ) ) {
		print_help ( argv[0] );
		exit ( 2 );
	}
	infile_name = argv[infile_index];
	outfile_name = argv[infile_index + 1];

	/* Open input and output files */
	infile = fopen ( infile_name, "r" );
	if ( ! infile ) {
		eprintf ( "Could not open %s for reading: %s\n",
			  infile_name, strerror ( errno ) );
		exit ( 1 );
	}
	outfile = fopen ( outfile_name, "w" );
	if ( ! outfile ) {
		eprintf ( "Could not open %s for writing: %s\n",
			  outfile_name, strerror ( errno ) );
		exit ( 1 );
	}

	/* Convert file */
	make_efi_rom ( infile, outfile, &opts );

	fclose ( outfile );
	fclose ( infile );

	return 0;
}

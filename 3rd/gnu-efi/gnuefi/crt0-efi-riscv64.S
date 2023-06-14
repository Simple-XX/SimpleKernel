/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copright (C) 2014 Linaro Ltd. <ard.biesheuvel@linaro.org>
 * Copright (C) 2018 Alexander Graf <agraf@suse.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice and this list of conditions, without modification.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 */

#ifndef EFI_SUBSYSTEM
#define EFI_SUBSYSTEM 10
#endif

	.section	.text.head

	/*
	 * Magic "MZ" signature for PE/COFF
	 */
	.globl	ImageBase
ImageBase:
	.ascii	"MZ"
	.skip	58				// 'MZ' + pad + offset == 64
	.long	pe_header - ImageBase		// Offset to the PE header.
pe_header:
	.ascii	"PE"
	.short 	0
coff_header:
	.short	0x5064				// riscv64
	.short	2				// nr_sections
	.long	0 				// TimeDateStamp
	.long	0				// PointerToSymbolTable
	.long	0				// NumberOfSymbols
	.short	section_table - optional_header	// SizeOfOptionalHeader
	.short	0x206				// Characteristics.
						// IMAGE_FILE_DEBUG_STRIPPED |
						// IMAGE_FILE_EXECUTABLE_IMAGE |
						// IMAGE_FILE_LINE_NUMS_STRIPPED
optional_header:
	.short	0x20b				// PE32+ format
	.byte	0x02				// MajorLinkerVersion
	.byte	0x14				// MinorLinkerVersion
	.long	_data - _start			// SizeOfCode
	.long	_data_size			// SizeOfInitializedData
	.long	0				// SizeOfUninitializedData
	.long	_start - ImageBase		// AddressOfEntryPoint
	.long	_start - ImageBase		// BaseOfCode

extra_header_fields:
	.quad	0				// ImageBase
	.long	0x1000				// SectionAlignment
	.long	0x200				// FileAlignment
	.short	0				// MajorOperatingSystemVersion
	.short	0				// MinorOperatingSystemVersion
	.short	0				// MajorImageVersion
	.short	0				// MinorImageVersion
	.short	0				// MajorSubsystemVersion
	.short	0				// MinorSubsystemVersion
	.long	0				// Win32VersionValue

	.long	_edata - ImageBase		// SizeOfImage

	// Everything before the kernel image is considered part of the header
	.long	_start - ImageBase		// SizeOfHeaders
	.long	0				// CheckSum
	.short	EFI_SUBSYSTEM			// Subsystem
	.short	0				// DllCharacteristics
	.quad	0				// SizeOfStackReserve
	.quad	0				// SizeOfStackCommit
	.quad	0				// SizeOfHeapReserve
	.quad	0				// SizeOfHeapCommit
	.long	0				// LoaderFlags
	.long	0x6				// NumberOfRvaAndSizes

	.quad	0				// ExportTable
	.quad	0				// ImportTable
	.quad	0				// ResourceTable
	.quad	0				// ExceptionTable
	.quad	0				// CertificationTable
	.quad	0				// BaseRelocationTable

	// Section table
section_table:
	/*
	 * The EFI application loader requires a relocation section
	 * because EFI applications must be relocatable.  This is a
	 * dummy section as far as we are concerned.
	 */
	.ascii	".reloc\0\0"
	.long	0
	.long	0
	.long	0				// SizeOfRawData
	.long	0				// PointerToRawData
	.long	0				// PointerToRelocations
	.long	0				// PointerToLineNumbers
	.short	0				// NumberOfRelocations
	.short	0				// NumberOfLineNumbers
	.long	0x42100040			// Characteristics (section flags)

	.ascii	".text\0\0\0"
	.long	_edata - _start			// VirtualSize
	.long	_start - ImageBase		// VirtualAddress
	.long	_edata - _start			// SizeOfRawData
	.long	_start - ImageBase		// PointerToRawData

	.long	0				// PointerToRelocations (0 for executables)
	.long	0				// PointerToLineNumbers (0 for executables)
	.short	0				// NumberOfRelocations  (0 for executables)
	.short	0				// NumberOfLineNumbers  (0 for executables)
	.long	0xe0500020			// Characteristics (section flags)

	.align	12
	.globl _start
_start:
	addi		sp, sp, -24
	sd		a0, 0(sp)
	sd		a1, 8(sp)
	sd		ra, 16(sp)
	lla		a0, ImageBase
	lla		a1, _DYNAMIC
	call		_relocate
	bne		a0, zero, 0f
	ld		a1, 8(sp)
	ld		a0, 0(sp)
	call		efi_main
	ld		ra, 16(sp)
0:	addi		sp, sp, 24
	ret

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif

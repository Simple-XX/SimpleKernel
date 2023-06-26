/*
 * crt0-efi-aarch64.S - PE/COFF header for AArch64 EFI applications
 *
 * Copright (C) 2014 Linaro Ltd. <ard.biesheuvel@linaro.org>
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

	.text
	.align		12

	.globl _start
_start:
	stp		x29, x30, [sp, #-32]!
	mov		x29, sp

	stp		x0, x1, [sp, #16]
	mov		x2, x0
	mov		x3, x1
	adr		x0, ImageBase
	adrp		x1, _DYNAMIC
	add		x1, x1, #:lo12:_DYNAMIC
	bl		_relocate
	cbnz		x0, 0f

	ldp		x0, x1, [sp, #16]
	bl		_entry

0:	ldp		x29, x30, [sp], #32
	ret

	// hand-craft a dummy .reloc section so EFI knows it's a relocatable executable:
 
 	.data
dummy:	.long	0

#define IMAGE_REL_ABSOLUTE	0
 	.section .reloc, "a"
label1:
	.long	dummy-label1				// Page RVA
	.long	12					// Block Size (2*4+2*2), must be aligned by 32 Bits
	.word	(IMAGE_REL_ABSOLUTE<<12) +  0		// reloc for dummy
	.word	(IMAGE_REL_ABSOLUTE<<12) +  0		// reloc for dummy

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif

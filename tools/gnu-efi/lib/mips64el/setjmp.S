/*
 * Copyright (c) 2006 - 2008, Intel Corporation. All rights reserved.
 * Copright (c) 2017 Lemote Co.
 * Author: Heiher <r@hev.cc>
 *
 * This program and the accompanying materials are licensed and made
available
 * under the terms and conditions of the BSD License which accompanies
this
 * distribution.  The full text of the license may be found at
 * http://opensource.org/licenses/bsd-license.php.
 *
 * THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
BASIS,
 * WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED.
 */
	.text
	.p2align 3

	.globl	setjmp
	.type	setjmp, @function
setjmp:
	sd	$ra, 0x00($a0)
	sd	$sp, 0x08($a0)
	sd	$fp, 0x10($a0)
	sd	$gp, 0x18($a0)

	sd	$s0, 0x20($a0)
	sd	$s1, 0x28($a0)
	sd	$s2, 0x30($a0)
	sd	$s3, 0x38($a0)
	sd	$s4, 0x40($a0)
	sd	$s5, 0x48($a0)
	sd	$s6, 0x50($a0)
	sd	$s7, 0x58($a0)

#ifdef	__mips_hard_float
	mfc0	$v0, $12
	ext	$v0, $v0, 29, 1
	beqz	$v0, 1f

	s.d	$f24, 0x60($a0)
	s.d	$f25, 0x68($a0)
	s.d	$f26, 0x70($a0)
	s.d	$f27, 0x78($a0)
	s.d	$f28, 0x80($a0)
	s.d	$f29, 0x88($a0)
	s.d	$f30, 0x90($a0)
	s.d	$f31, 0x98($a0)

1:
#endif
	move	$v0, $zero
	jr	$ra

	.globl	longjmp
	.type	longjmp, @function
longjmp:
	ld	$ra, 0x00($a0)
	ld	$sp, 0x08($a0)
	ld	$fp, 0x10($a0)
	ld	$gp, 0x18($a0)

	ld	$s0, 0x20($a0)
	ld	$s1, 0x28($a0)
	ld	$s2, 0x30($a0)
	ld	$s3, 0x38($a0)
	ld	$s4, 0x40($a0)
	ld	$s5, 0x48($a0)
	ld	$s6, 0x50($a0)
	ld	$s7, 0x58($a0)

#ifdef	__mips_hard_float
	mfc0	$v0, $12
	ext	$v0, $v0, 29, 1
	beqz	$v0, 1f

	l.d	$f24, 0x60($a0)
	l.d	$f25, 0x68($a0)
	l.d	$f26, 0x70($a0)
	l.d	$f27, 0x78($a0)
	l.d	$f28, 0x80($a0)
	l.d	$f29, 0x88($a0)
	l.d	$f30, 0x90($a0)
	l.d	$f31, 0x98($a0)

1:
#endif
	li	$v0, 1
	movn	$v0, $a1, $a1
	jr	$ra

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif

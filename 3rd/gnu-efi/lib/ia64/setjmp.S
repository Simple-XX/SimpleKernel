/*
 * Copyright (c) 2006 - 2008, Intel Corporation. All rights reserved.
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
	.globl	setjmp
	.type	setjmp, @function
setjmp:
	alloc	loc0 = ar.pfs, 1, 2, 1, 0
	;;
	mov	r14 = ar.unat
	mov	r15 = ar.bsp
	add	r10 = 0x10*20, in0
	;;
	stf.spill.nta	[in0] = f2, 0x10 
	st8.spill.nta	[r10] = r4, 8
	mov		r21 = b1
	;;
	stf.spill.nta	[in0] = f3, 0x10 
	st8.spill.nta	[r10] = r5, 8
	mov		r22 = b2
	;;
	stf.spill.nta	[in0] = f4, 0x10 
	st8.spill.nta	[r10] = r6, 8
	mov		r23 = b3
	;;
	stf.spill.nta	[in0] = f5, 0x10 
	st8.spill.nta	[r10] = r7, 8
	mov		r24 = b4
	;;
	stf.spill.nta	[in0] = f16, 0x10 
	st8.spill.nta	[r10] = sp, 8
	mov		r25 = b5
	;;
	stf.spill.nta	[in0] = f17, 0x10 
	st8.nta		[r10] = loc1, 8
	mov		r16 = pr
	;;
	stf.spill.nta	[in0] = f18, 0x10 
	st8.nta		[r10] = r21, 8
	mov		r17 = ar.lc
	;;
	stf.spill.nta	[in0] = f19, 0x10 
	st8.nta		[r10] = r22, 8
	;;
	stf.spill.nta	[in0] = f20, 0x10 
	st8.nta		[r10] = r23, 8
	;;
	stf.spill.nta	[in0] = f21, 0x10 
	st8.nta		[r10] = r24, 8
	;;
	stf.spill.nta	[in0] = f22, 0x10 
	st8.nta		[r10] = r25, 8
	;;
	stf.spill.nta	[in0] = f23, 0x10 
	mov		r18 = ar.unat
	;;
	stf.spill.nta	[in0] = f24, 0x10 
	st8.nta		[r10] = r14, 8
	;;
	stf.spill.nta	[in0] = f25, 0x10 
	st8.nta		[r10] = r18, 8
	;;
	stf.spill.nta	[in0] = f26, 0x10 
	st8.nta		[r10] = loc0, 8
	;;
	stf.spill.nta	[in0] = f27, 0x10 
	st8.nta		[r10] = r15, 8
	mov		r8 = 0
	;;
	stf.spill.nta	[in0] = f28, 0x10 
	mov		r19 = ar.fpsr
	;;
	stf.spill.nta	[in0] = f29, 0x10 
	st8.nta		[r10] = r16, 8
	mov		ar.pfs = loc0
	;;
	stf.spill.nta	[in0] = f30, 0x10 
	st8.nta		[r10] = r17, 8
	mov		b0 = loc1
	;;
	stf.spill.nta	[in0] = f31, 0x10 
	st8.nta		[r10] = r19
	;;
	mov		ar.unat = r14
	br.ret.sptk	b0
	;;

	.globl	longjmp
	.type	longjmp, @function
	.regstk 2, 0, 0, 0
longjmp:
	add		r10 = 0x10*20 + 8*14, in0
	movl		r2  = ~((((1<<14) - 1) << 16) | 3)
	;;
	ld8.nt1		r14 = [r10], -8*2
	mov		r15 = ar.bspstore
	;;
	ld8.nt1		r17 = [r10], -8
	mov		r16 = ar.rsc
	cmp.leu		p6  = r14, r15
	;;
	ld8.nt1		r18 = [r10], -8
	ld8.nt1		r25 = [r10], -8
	and		r2  = r16, r2
	;;
	ldf.fill.nt1	f2  = [in0], 0x10
	ld8.nt1		r24 = [r10], -8
	mov		b5  = r25
	;;
	mov		ar.rsc = r2
	ld8.nt1		r23 = [r10], -8
	mov		b4 = r24
	;;
	ldf.fill.nt1	f3 = [in0], 0x10
	mov		ar.unat = r17
(p6)	br.spnt.many	_skip_flushrs
	;;
	flushrs
	mov		r15 = ar.bsp
	;;
_skip_flushrs:
	mov		r31 = ar.rnat
	loadrs
	;;
	ldf.fill.nt1	f4  = [in0], 0x10
	ld8.nt1		r22 = [r10], -8
	dep		r2  = -1, r14, 3, 6
	;;
	ldf.fill.nt1	f5  = [in0], 0x10
	ld8.nt1		r21 = [r10], -8
	cmp.ltu		p6  = r2, r15
	;;
	ld8.nt1		r20 = [r10], -0x10
(p6)	ld8.nta		r31 = [r2]
	mov		b3  = r23
	;;
	ldf.fill.nt1	f16 = [in0], 0x10
	ld8.fill.nt1	r7  = [r10], -8
	mov		b2  = r22
	;;
	ldf.fill.nt1	f17 = [in0], 0x10
	ld8.fill.nt1	r6  = [r10], -8
	mov		b1  = r21
	;;
	ldf.fill.nt1	f18 = [in0], 0x10
	ld8.fill.nt1	r5  = [r10], -8
	mov		b0  = r20
	;;
	ldf.fill.nt1	f19 = [in0], 0x10
	ld8.fill.nt1	r4  = [r10], 8*13
	;;
	ldf.fill.nt1	f20 = [in0], 0x10
	ld8.nt1		r19 = [r10], 0x10
	;;
	ldf.fill.nt1	f21 = [in0], 0x10
	ld8.nt1		r26 = [r10], 8
	mov		ar.pfs = r19
	;;
	ldf.fill.nt1	f22 = [in0], 0x10
	ld8.nt1		r27 = [r10], 8
	mov		pr  = r26, -1
	;;
	ldf.fill.nt1	f23 = [in0], 0x10
	ld8.nt1		r28 = [r10], -17*8 - 0x10
	mov		ar.lc = r27
	;;
	ldf.fill.nt1	f24 = [in0], 0x10
	ldf.fill.nt1	f25 = [in0], 0x10
	mov		r8  = in1
	;;
	ldf.fill.nt1	f26 = [in0], 0x10
	ldf.fill.nt1	f31 = [r10], -0x10
	;;
	ldf.fill.nt1	f27 = [in0], 0x10
	ldf.fill.nt1	f30 = [r10], -0x10
	;;
	ldf.fill.nt1	f28 = [in0]
	ldf.fill.nt1	f29 = [r10], 0x10*3 + 8*4
	;;
	ld8.fill.nt1	sp  = [r10]
	mov		ar.unat = r18
	;;
	mov		ar.bspstore = r14
	mov		ar.rnat = r31
	;;
	invala
	mov		ar.rsc = r16
	br.ret.sptk	b0

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif

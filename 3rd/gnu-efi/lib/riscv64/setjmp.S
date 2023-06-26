// SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause
/*
 * Copyright Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

	.text
	.p2align 3

#define GREG_LIST \
	REG_ONE(s0, 0); \
	REG_ONE(s1, 8); \
	REG_ONE(s2, 16); \
	REG_ONE(s3, 24); \
	REG_ONE(s4, 32); \
	REG_ONE(s5, 40); \
	REG_ONE(s6, 48); \
	REG_ONE(s7, 56); \
	REG_ONE(s8, 64); \
	REG_ONE(s9, 72); \
	REG_ONE(s10, 80); \
	REG_ONE(s11, 88); \
	REG_ONE(sp, 96); \
	REG_ONE(ra, 104);

#define FREG_LIST \
	FREG_ONE(fs0,  112); \
	FREG_ONE(fs1,  120); \
	FREG_ONE(fs2,  128); \
	FREG_ONE(fs3,  136); \
	FREG_ONE(fs4,  144); \
	FREG_ONE(fs5,  152); \
	FREG_ONE(fs6,  160); \
	FREG_ONE(fs7,  168); \
	FREG_ONE(fs8,  176); \
	FREG_ONE(fs9,  184); \
	FREG_ONE(fs10, 192); \
	FREG_ONE(fs11, 200);

#define REG_ONE(R, P) sd R, P(a0)
#define FREG_ONE(R, P) fsd R, P(a0)

	.globl setjmp
	.type setjmp, @function

setjmp:
	GREG_LIST
#ifndef __riscv_float_abi_soft
	FREG_LIST
#endif
	li a0, 0
	ret

#undef REG_ONE
#undef FREG_ONE

#define REG_ONE(R, P) ld R, P(a0)
#define FREG_ONE(R, P) fld R, P(a0)

	.globl longjmp
	.type longjmp, @function

longjmp:
	GREG_LIST
#ifndef __riscv_float_abi_soft
	FREG_LIST
#endif
	seqz a0, a1
	add a0, a0, a1
	ret

#if defined(__ELF__) && defined(__linux__)
	.section .note.GNU-stack,"",%progbits
#endif

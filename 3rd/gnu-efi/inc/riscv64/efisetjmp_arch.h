/* SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause */

#ifndef GNU_EFI_RISCV64_SETJMP_H
#define GNU_EFI_RISCV64_SETJMP_H

#define JMPBUF_ALIGN 8

typedef struct {
	/* GP regs */
	UINT64	s0;
	UINT64	s1;
	UINT64	s2;
	UINT64	s3;
	UINT64	s4;
	UINT64	s5;
	UINT64	s6;
	UINT64	s7;
	UINT64	s8;
	UINT64	s9;
	UINT64	s10;
	UINT64	s11;
	UINT64	sp;
	UINT64	ra;

	/* FP regs */
	UINT64	fs0;
	UINT64	fs1;
	UINT64	fs2;
	UINT64	fs3;
	UINT64	fs4;
	UINT64	fs5;
	UINT64	fs6;
	UINT64	fs7;
	UINT64	fs8;
	UINT64	fs9;
	UINT64	fs10;
	UINT64	fs11;
} ALIGN(JMPBUF_ALIGN) jmp_buf[1];

#endif /* GNU_EFI_RISCV64_SETJMP_H */

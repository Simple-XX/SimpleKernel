/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_ecall_interface.h>

#define SBI_ECALL(__eid, __fid, __a0, __a1, __a2)                             \
	({                                                                    \
		register unsigned long a0 asm("a0") = (unsigned long)(__a0);  \
		register unsigned long a1 asm("a1") = (unsigned long)(__a1);  \
		register unsigned long a2 asm("a2") = (unsigned long)(__a2);  \
		register unsigned long a6 asm("a6") = (unsigned long)(__fid); \
		register unsigned long a7 asm("a7") = (unsigned long)(__eid); \
		asm volatile("ecall"                                          \
			     : "+r"(a0)                                       \
			     : "r"(a1), "r"(a2), "r"(a6), "r"(a7)             \
			     : "memory");                                     \
		a0;                                                           \
	})

#define SBI_ECALL_0(__eid, __fid) SBI_ECALL(__eid, __fid, 0, 0, 0)
#define SBI_ECALL_1(__eid, __fid, __a0) SBI_ECALL(__eid, __fid, __a0, 0, 0)
#define SBI_ECALL_2(__eid, __fid, __a0, __a1) SBI_ECALL(__eid, __fid, __a0, __a1, 0)

#define sbi_ecall_console_putc(c) SBI_ECALL_1(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0, (c))

static inline void sbi_ecall_console_puts(const char *str)
{
	while (str && *str)
		sbi_ecall_console_putc(*str++);
}

#define wfi()                                             \
	do {                                              \
		__asm__ __volatile__("wfi" ::: "memory"); \
	} while (0)

void test_main(unsigned long a0, unsigned long a1)
{
	sbi_ecall_console_puts("\nTest payload running\n");

	while (1)
		wfi();
}

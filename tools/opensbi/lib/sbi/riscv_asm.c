/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_console.h>

/* determine CPU extension, return non-zero support */
int misa_extension_imp(char ext)
{
	unsigned long misa = csr_read(CSR_MISA);

	if (misa) {
		if ('A' <= ext && ext <= 'Z')
			return misa & (1 << (ext - 'A'));
		if ('a' <= ext && ext <= 'z')
			return misa & (1 << (ext - 'a'));
		return 0;
	}

	return sbi_platform_misa_extension(sbi_platform_thishart_ptr(), ext);
}

int misa_xlen(void)
{
	long r;

	if (csr_read(CSR_MISA) == 0)
		return sbi_platform_misa_xlen(sbi_platform_thishart_ptr());

	__asm__ __volatile__(
		"csrr   t0, misa\n\t"
		"slti   t1, t0, 0\n\t"
		"slli   t1, t1, 1\n\t"
		"slli   t0, t0, 1\n\t"
		"slti   t0, t0, 0\n\t"
		"add    %0, t0, t1"
		: "=r"(r)
		:
		: "t0", "t1");

	return r ? r : -1;
}

void misa_string(int xlen, char *out, unsigned int out_sz)
{
	unsigned int i, pos = 0;
	const char valid_isa_order[] = "iemafdqclbjtpvnhkorwxyzg";

	if (!out)
		return;

	if (5 <= (out_sz - pos)) {
		out[pos++] = 'r';
		out[pos++] = 'v';
		switch (xlen) {
		case 1:
			out[pos++] = '3';
			out[pos++] = '2';
			break;
		case 2:
			out[pos++] = '6';
			out[pos++] = '4';
			break;
		case 3:
			out[pos++] = '1';
			out[pos++] = '2';
			out[pos++] = '8';
			break;
		default:
			sbi_panic("%s: Unknown misa.MXL encoding %d",
				   __func__, xlen);
			return;
		}
	}

	for (i = 0; i < array_size(valid_isa_order) && (pos < out_sz); i++) {
		if (misa_extension_imp(valid_isa_order[i]))
			out[pos++] = valid_isa_order[i];
	}

	if (pos < out_sz)
		out[pos++] = '\0';
}

unsigned long csr_read_num(int csr_num)
{
#define switchcase_csr_read(__csr_num, __val)		\
	case __csr_num:					\
		__val = csr_read(__csr_num);		\
		break;
#define switchcase_csr_read_2(__csr_num, __val)	\
	switchcase_csr_read(__csr_num + 0, __val)	\
	switchcase_csr_read(__csr_num + 1, __val)
#define switchcase_csr_read_4(__csr_num, __val)	\
	switchcase_csr_read_2(__csr_num + 0, __val)	\
	switchcase_csr_read_2(__csr_num + 2, __val)
#define switchcase_csr_read_8(__csr_num, __val)	\
	switchcase_csr_read_4(__csr_num + 0, __val)	\
	switchcase_csr_read_4(__csr_num + 4, __val)
#define switchcase_csr_read_16(__csr_num, __val)	\
	switchcase_csr_read_8(__csr_num + 0, __val)	\
	switchcase_csr_read_8(__csr_num + 8, __val)
#define switchcase_csr_read_32(__csr_num, __val)	\
	switchcase_csr_read_16(__csr_num + 0, __val)	\
	switchcase_csr_read_16(__csr_num + 16, __val)
#define switchcase_csr_read_64(__csr_num, __val)	\
	switchcase_csr_read_32(__csr_num + 0, __val)	\
	switchcase_csr_read_32(__csr_num + 32, __val)

	unsigned long ret = 0;

	switch (csr_num) {
	switchcase_csr_read_16(CSR_PMPCFG0, ret)
	switchcase_csr_read_64(CSR_PMPADDR0, ret)
	switchcase_csr_read(CSR_MCYCLE, ret)
	switchcase_csr_read(CSR_MINSTRET, ret)
	switchcase_csr_read(CSR_MHPMCOUNTER3, ret)
	switchcase_csr_read_4(CSR_MHPMCOUNTER4, ret)
	switchcase_csr_read_8(CSR_MHPMCOUNTER8, ret)
	switchcase_csr_read_16(CSR_MHPMCOUNTER16, ret)
	switchcase_csr_read(CSR_MCOUNTINHIBIT, ret)
	switchcase_csr_read(CSR_MHPMEVENT3, ret)
	switchcase_csr_read_4(CSR_MHPMEVENT4, ret)
	switchcase_csr_read_8(CSR_MHPMEVENT8, ret)
	switchcase_csr_read_16(CSR_MHPMEVENT16, ret)
#if __riscv_xlen == 32
	switchcase_csr_read(CSR_MCYCLEH, ret)
	switchcase_csr_read(CSR_MINSTRETH, ret)
	switchcase_csr_read(CSR_MHPMCOUNTER3H, ret)
	switchcase_csr_read_4(CSR_MHPMCOUNTER4H, ret)
	switchcase_csr_read_8(CSR_MHPMCOUNTER8H, ret)
	switchcase_csr_read_16(CSR_MHPMCOUNTER16H, ret)
	/**
	 * The CSR range MHPMEVENT[3-16]H are available only if sscofpmf
	 * extension is present. The caller must ensure that.
	 */
	switchcase_csr_read(CSR_MHPMEVENT3H, ret)
	switchcase_csr_read_4(CSR_MHPMEVENT4H, ret)
	switchcase_csr_read_8(CSR_MHPMEVENT8H, ret)
	switchcase_csr_read_16(CSR_MHPMEVENT16H, ret)
#endif

	default:
		sbi_panic("%s: Unknown CSR %#x", __func__, csr_num);
		break;
	};

	return ret;

#undef switchcase_csr_read_64
#undef switchcase_csr_read_32
#undef switchcase_csr_read_16
#undef switchcase_csr_read_8
#undef switchcase_csr_read_4
#undef switchcase_csr_read_2
#undef switchcase_csr_read
}

void csr_write_num(int csr_num, unsigned long val)
{
#define switchcase_csr_write(__csr_num, __val)		\
	case __csr_num:					\
		csr_write(__csr_num, __val);		\
		break;
#define switchcase_csr_write_2(__csr_num, __val)	\
	switchcase_csr_write(__csr_num + 0, __val)	\
	switchcase_csr_write(__csr_num + 1, __val)
#define switchcase_csr_write_4(__csr_num, __val)	\
	switchcase_csr_write_2(__csr_num + 0, __val)	\
	switchcase_csr_write_2(__csr_num + 2, __val)
#define switchcase_csr_write_8(__csr_num, __val)	\
	switchcase_csr_write_4(__csr_num + 0, __val)	\
	switchcase_csr_write_4(__csr_num + 4, __val)
#define switchcase_csr_write_16(__csr_num, __val)	\
	switchcase_csr_write_8(__csr_num + 0, __val)	\
	switchcase_csr_write_8(__csr_num + 8, __val)
#define switchcase_csr_write_32(__csr_num, __val)	\
	switchcase_csr_write_16(__csr_num + 0, __val)	\
	switchcase_csr_write_16(__csr_num + 16, __val)
#define switchcase_csr_write_64(__csr_num, __val)	\
	switchcase_csr_write_32(__csr_num + 0, __val)	\
	switchcase_csr_write_32(__csr_num + 32, __val)

	switch (csr_num) {
	switchcase_csr_write_16(CSR_PMPCFG0, val)
	switchcase_csr_write_64(CSR_PMPADDR0, val)
	switchcase_csr_write(CSR_MCYCLE, val)
	switchcase_csr_write(CSR_MINSTRET, val)
	switchcase_csr_write(CSR_MHPMCOUNTER3, val)
	switchcase_csr_write_4(CSR_MHPMCOUNTER4, val)
	switchcase_csr_write_8(CSR_MHPMCOUNTER8, val)
	switchcase_csr_write_16(CSR_MHPMCOUNTER16, val)
#if __riscv_xlen == 32
	switchcase_csr_write(CSR_MCYCLEH, val)
	switchcase_csr_write(CSR_MINSTRETH, val)
	switchcase_csr_write(CSR_MHPMCOUNTER3H, val)
	switchcase_csr_write_4(CSR_MHPMCOUNTER4H, val)
	switchcase_csr_write_8(CSR_MHPMCOUNTER8H, val)
	switchcase_csr_write_16(CSR_MHPMCOUNTER16H, val)
	switchcase_csr_write(CSR_MHPMEVENT3H, val)
	switchcase_csr_write_4(CSR_MHPMEVENT4H, val)
	switchcase_csr_write_8(CSR_MHPMEVENT8H, val)
	switchcase_csr_write_16(CSR_MHPMEVENT16H, val)
#endif
	switchcase_csr_write(CSR_MCOUNTINHIBIT, val)
	switchcase_csr_write(CSR_MHPMEVENT3, val)
	switchcase_csr_write_4(CSR_MHPMEVENT4, val)
	switchcase_csr_write_8(CSR_MHPMEVENT8, val)
	switchcase_csr_write_16(CSR_MHPMEVENT16, val)

	default:
		sbi_panic("%s: Unknown CSR %#x", __func__, csr_num);
		break;
	};

#undef switchcase_csr_write_64
#undef switchcase_csr_write_32
#undef switchcase_csr_write_16
#undef switchcase_csr_write_8
#undef switchcase_csr_write_4
#undef switchcase_csr_write_2
#undef switchcase_csr_write
}

static unsigned long ctz(unsigned long x)
{
	unsigned long ret = 0;

	if (x == 0)
		return 8 * sizeof(x);

	while (!(x & 1UL)) {
		ret++;
		x = x >> 1;
	}

	return ret;
}

int pmp_set(unsigned int n, unsigned long prot, unsigned long addr,
	    unsigned long log2len)
{
	int pmpcfg_csr, pmpcfg_shift, pmpaddr_csr;
	unsigned long cfgmask, pmpcfg;
	unsigned long addrmask, pmpaddr;

	/* check parameters */
	if (n >= PMP_COUNT || log2len > __riscv_xlen || log2len < PMP_SHIFT)
		return SBI_EINVAL;

	/* calculate PMP register and offset */
#if __riscv_xlen == 32
	pmpcfg_csr   = CSR_PMPCFG0 + (n >> 2);
	pmpcfg_shift = (n & 3) << 3;
#elif __riscv_xlen == 64
	pmpcfg_csr   = (CSR_PMPCFG0 + (n >> 2)) & ~1;
	pmpcfg_shift = (n & 7) << 3;
#else
# error "Unexpected __riscv_xlen"
#endif
	pmpaddr_csr = CSR_PMPADDR0 + n;

	/* encode PMP config */
	prot &= ~PMP_A;
	prot |= (log2len == PMP_SHIFT) ? PMP_A_NA4 : PMP_A_NAPOT;
	cfgmask = ~(0xffUL << pmpcfg_shift);
	pmpcfg	= (csr_read_num(pmpcfg_csr) & cfgmask);
	pmpcfg |= ((prot << pmpcfg_shift) & ~cfgmask);

	/* encode PMP address */
	if (log2len == PMP_SHIFT) {
		pmpaddr = (addr >> PMP_SHIFT);
	} else {
		if (log2len == __riscv_xlen) {
			pmpaddr = -1UL;
		} else {
			addrmask = (1UL << (log2len - PMP_SHIFT)) - 1;
			pmpaddr	 = ((addr >> PMP_SHIFT) & ~addrmask);
			pmpaddr |= (addrmask >> 1);
		}
	}

	/* write csrs */
	csr_write_num(pmpaddr_csr, pmpaddr);
	csr_write_num(pmpcfg_csr, pmpcfg);

	return 0;
}

int pmp_get(unsigned int n, unsigned long *prot_out, unsigned long *addr_out,
	    unsigned long *log2len)
{
	int pmpcfg_csr, pmpcfg_shift, pmpaddr_csr;
	unsigned long cfgmask, pmpcfg, prot;
	unsigned long t1, addr, len;

	/* check parameters */
	if (n >= PMP_COUNT || !prot_out || !addr_out || !log2len)
		return SBI_EINVAL;
	*prot_out = *addr_out = *log2len = 0;

	/* calculate PMP register and offset */
#if __riscv_xlen == 32
	pmpcfg_csr   = CSR_PMPCFG0 + (n >> 2);
	pmpcfg_shift = (n & 3) << 3;
#elif __riscv_xlen == 64
	pmpcfg_csr   = (CSR_PMPCFG0 + (n >> 2)) & ~1;
	pmpcfg_shift = (n & 7) << 3;
#else
# error "Unexpected __riscv_xlen"
#endif
	pmpaddr_csr = CSR_PMPADDR0 + n;

	/* decode PMP config */
	cfgmask = (0xffUL << pmpcfg_shift);
	pmpcfg	= csr_read_num(pmpcfg_csr) & cfgmask;
	prot	= pmpcfg >> pmpcfg_shift;

	/* decode PMP address */
	if ((prot & PMP_A) == PMP_A_NAPOT) {
		addr = csr_read_num(pmpaddr_csr);
		if (addr == -1UL) {
			addr	= 0;
			len	= __riscv_xlen;
		} else {
			t1	= ctz(~addr);
			addr	= (addr & ~((1UL << t1) - 1)) << PMP_SHIFT;
			len	= (t1 + PMP_SHIFT + 1);
		}
	} else {
		addr	= csr_read_num(pmpaddr_csr) << PMP_SHIFT;
		len	= PMP_SHIFT;
	}

	/* return details */
	*prot_out    = prot;
	*addr_out    = addr;
	*log2len     = len;

	return 0;
}

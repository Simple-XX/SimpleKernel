/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_io.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_irqchip.h>
#include <sbi/sbi_error.h>
#include <sbi_utils/irqchip/imsic.h>

#define IMSIC_MMIO_PAGE_LE		0x00
#define IMSIC_MMIO_PAGE_BE		0x04

#define IMSIC_MIN_ID			63
#define IMSIC_MAX_ID			2047

#define IMSIC_EIDELIVERY		0x70

#define IMSIC_EITHRESHOLD		0x72

#define IMSIC_TOPEI			0x76
#define IMSIC_TOPEI_ID_SHIFT		16
#define IMSIC_TOPEI_ID_MASK		0x7ff
#define IMSIC_TOPEI_PRIO_MASK		0x7ff

#define IMSIC_EIP0			0x80

#define IMSIC_EIP63			0xbf

#define IMSIC_EIPx_BITS			32

#define IMSIC_EIE0			0xc0

#define IMSIC_EIE63			0xff

#define IMSIC_EIEx_BITS			32

#define IMSIC_DISABLE_EIDELIVERY	0
#define IMSIC_ENABLE_EIDELIVERY		1
#define IMSIC_DISABLE_EITHRESHOLD	1
#define IMSIC_ENABLE_EITHRESHOLD	0

#define IMSIC_IPI_ID			1

#define imsic_csr_write(__c, __v)	\
do { \
	csr_write(CSR_MISELECT, __c); \
	csr_write(CSR_MIREG, __v); \
} while (0)

#define imsic_csr_read(__c)	\
({ \
	unsigned long __v; \
	csr_write(CSR_MISELECT, __c); \
	__v = csr_read(CSR_MIREG); \
	__v; \
})

#define imsic_csr_set(__c, __v)		\
do { \
	csr_write(CSR_MISELECT, __c); \
	csr_set(CSR_MIREG, __v); \
} while (0)

#define imsic_csr_clear(__c, __v)	\
do { \
	csr_write(CSR_MISELECT, __c); \
	csr_clear(CSR_MIREG, __v); \
} while (0)

static struct imsic_data *imsic_hartid2data[SBI_HARTMASK_MAX_BITS];
static int imsic_hartid2file[SBI_HARTMASK_MAX_BITS];

int imsic_map_hartid_to_data(u32 hartid, struct imsic_data *imsic, int file)
{
	if (!imsic || !imsic->targets_mmode ||
	    (SBI_HARTMASK_MAX_BITS <= hartid))
		return SBI_EINVAL;

	imsic_hartid2data[hartid] = imsic;
	imsic_hartid2file[hartid] = file;
	return 0;
}

struct imsic_data *imsic_get_data(u32 hartid)
{
	if (SBI_HARTMASK_MAX_BITS <= hartid)
		return NULL;
	return imsic_hartid2data[hartid];
}

int imsic_get_target_file(u32 hartid)
{
	if ((SBI_HARTMASK_MAX_BITS <= hartid) ||
	    !imsic_hartid2data[hartid])
		return SBI_ENOENT;
	return imsic_hartid2file[hartid];
}

static int imsic_external_irqfn(struct sbi_trap_regs *regs)
{
	ulong mirq;

	while ((mirq = csr_swap(CSR_MTOPEI, 0))) {
		mirq = (mirq >> IMSIC_TOPEI_ID_SHIFT);

		switch (mirq) {
		case IMSIC_IPI_ID:
			sbi_ipi_process();
			break;
		default:
			sbi_printf("%s: unhandled IRQ%d\n",
				   __func__, (u32)mirq);
			break;
		}
	}

	return 0;
}

static void imsic_ipi_send(u32 target_hart)
{
	unsigned long reloff;
	struct imsic_regs *regs;
	struct imsic_data *data = imsic_hartid2data[target_hart];
	int file = imsic_hartid2file[target_hart];

	if (!data || !data->targets_mmode)
		return;

	regs = &data->regs[0];
	reloff = file * (1UL << data->guest_index_bits) * IMSIC_MMIO_PAGE_SZ;
	while (regs->size && (regs->size <= reloff)) {
		reloff -= regs->size;
		regs++;
	}

	if (regs->size && (reloff < regs->size))
		writel(IMSIC_IPI_ID,
		       (void *)(regs->addr + reloff + IMSIC_MMIO_PAGE_LE));
}

static struct sbi_ipi_device imsic_ipi_device = {
	.name		= "aia-imsic",
	.ipi_send	= imsic_ipi_send
};

static void imsic_local_eix_update(unsigned long base_id,
				   unsigned long num_id, bool pend, bool val)
{
	unsigned long i, isel, ireg;
	unsigned long id = base_id, last_id = base_id + num_id;

	while (id < last_id) {
		isel = id / __riscv_xlen;
		isel *= __riscv_xlen / IMSIC_EIPx_BITS;
		isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

		ireg = 0;
		for (i = id & (__riscv_xlen - 1);
		     (id < last_id) && (i < __riscv_xlen); i++) {
			ireg |= BIT(i);
			id++;
		}

		if (val)
			imsic_csr_set(isel, ireg);
		else
			imsic_csr_clear(isel, ireg);
	}
}

void imsic_local_irqchip_init(void)
{
	/*
	 * This function is expected to be called from:
	 * 1) nascent_init() platform callback which is called
	 *    very early on each HART in boot-up path and and
	 *    HSM resume path.
	 * 2) irqchip_init() platform callback which is called
	 *    in boot-up path.
	 */

	/* Setup threshold to allow all enabled interrupts */
	imsic_csr_write(IMSIC_EITHRESHOLD, IMSIC_ENABLE_EITHRESHOLD);

	/* Enable interrupt delivery */
	imsic_csr_write(IMSIC_EIDELIVERY, IMSIC_ENABLE_EIDELIVERY);

	/* Enable IPI */
	imsic_local_eix_update(IMSIC_IPI_ID, 1, false, true);
}

int imsic_warm_irqchip_init(void)
{
	struct imsic_data *imsic = imsic_hartid2data[current_hartid()];

	/* Sanity checks */
	if (!imsic || !imsic->targets_mmode)
		return SBI_EINVAL;

	/* Disable all interrupts */
	imsic_local_eix_update(1, imsic->num_ids, false, false);

	/* Clear IPI pending */
	imsic_local_eix_update(IMSIC_IPI_ID, 1, true, false);

	/* Local IMSIC initialization */
	imsic_local_irqchip_init();

	return 0;
}

int imsic_data_check(struct imsic_data *imsic)
{
	u32 i, tmp;
	unsigned long base_addr, addr, mask;

	/* Sanity checks */
	if (!imsic ||
	    (imsic->num_ids < IMSIC_MIN_ID) ||
	    (IMSIC_MAX_ID < imsic->num_ids))
		return SBI_EINVAL;

	tmp = BITS_PER_LONG - IMSIC_MMIO_PAGE_SHIFT;
	if (tmp < imsic->guest_index_bits)
		return SBI_EINVAL;

	tmp = BITS_PER_LONG - IMSIC_MMIO_PAGE_SHIFT -
	      imsic->guest_index_bits;
	if (tmp < imsic->hart_index_bits)
		return SBI_EINVAL;

	tmp = BITS_PER_LONG - IMSIC_MMIO_PAGE_SHIFT -
	      imsic->guest_index_bits - imsic->hart_index_bits;
	if (tmp < imsic->group_index_bits)
		return SBI_EINVAL;

	tmp = IMSIC_MMIO_PAGE_SHIFT + imsic->guest_index_bits +
	      imsic->hart_index_bits;
	if (imsic->group_index_shift < tmp)
		return SBI_EINVAL;
	tmp = imsic->group_index_bits + imsic->group_index_shift - 1;
	if (tmp >= BITS_PER_LONG)
		return SBI_EINVAL;

	/*
	 * Number of interrupt identities should be 1 less than
	 * multiple of 63
	 */
	if ((imsic->num_ids & IMSIC_MIN_ID) != IMSIC_MIN_ID)
		return SBI_EINVAL;

	/* We should have at least one regset */
	if (!imsic->regs[0].size)
		return SBI_EINVAL;

	/* Match patter of each regset */
	base_addr = imsic->regs[0].addr;
	base_addr &= ~((1UL << (imsic->guest_index_bits +
				 imsic->hart_index_bits +
				 IMSIC_MMIO_PAGE_SHIFT)) - 1);
	base_addr &= ~(((1UL << imsic->group_index_bits) - 1) <<
			imsic->group_index_shift);
	for (i = 0; i < IMSIC_MAX_REGS && imsic->regs[i].size; i++) {
		mask = (1UL << imsic->guest_index_bits) * IMSIC_MMIO_PAGE_SZ;
		mask -= 1UL;
		if (imsic->regs[i].size & mask)
			return SBI_EINVAL;

		addr = imsic->regs[i].addr;
		addr &= ~((1UL << (imsic->guest_index_bits +
					 imsic->hart_index_bits +
					 IMSIC_MMIO_PAGE_SHIFT)) - 1);
		addr &= ~(((1UL << imsic->group_index_bits) - 1) <<
				imsic->group_index_shift);
		if (base_addr != addr)
			return SBI_EINVAL;
	}

	return 0;
}

int imsic_cold_irqchip_init(struct imsic_data *imsic)
{
	int i, rc;
	struct sbi_domain_memregion reg;

	/* Sanity checks */
	rc = imsic_data_check(imsic);
	if (rc)
		return rc;

	/* We only initialize M-mode IMSIC */
	if (!imsic->targets_mmode)
		return SBI_EINVAL;

	/* Setup external interrupt function for IMSIC */
	sbi_irqchip_set_irqfn(imsic_external_irqfn);

	/* Add IMSIC regions to the root domain */
	for (i = 0; i < IMSIC_MAX_REGS && imsic->regs[i].size; i++) {
		sbi_domain_memregion_init(imsic->regs[i].addr,
					  imsic->regs[i].size,
					  SBI_DOMAIN_MEMREGION_MMIO, &reg);
		rc = sbi_domain_root_add_memregion(&reg);
		if (rc)
			return rc;
	}

	/* Register IPI device */
	sbi_ipi_set_device(&imsic_ipi_device);

	return 0;
}

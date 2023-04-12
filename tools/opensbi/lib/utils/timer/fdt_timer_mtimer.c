/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <libfdt.h>
#include <sbi/sbi_error.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/timer/fdt_timer.h>
#include <sbi_utils/timer/aclint_mtimer.h>

#define MTIMER_MAX_NR			16

struct timer_mtimer_quirks {
	unsigned int	mtime_offset;
	bool		has_64bit_mmio;
	bool		without_mtime;
};

static unsigned long mtimer_count = 0;
static struct aclint_mtimer_data mtimer[MTIMER_MAX_NR];
static struct aclint_mtimer_data *mt_reference = NULL;

static int timer_mtimer_cold_init(void *fdt, int nodeoff,
				  const struct fdt_match *match)
{
	int i, rc;
	unsigned long addr[2], size[2];
	struct aclint_mtimer_data *mt;

	if (MTIMER_MAX_NR <= mtimer_count)
		return SBI_ENOSPC;
	mt = &mtimer[mtimer_count];

	rc = fdt_parse_aclint_node(fdt, nodeoff, true,
				   &addr[0], &size[0], &addr[1], &size[1],
				   &mt->first_hartid, &mt->hart_count);
	if (rc)
		return rc;
	mt->has_64bit_mmio = true;
	mt->has_shared_mtime = false;

	rc = fdt_parse_timebase_frequency(fdt, &mt->mtime_freq);
	if (rc)
		return rc;

	if (match->data) { /* SiFive CLINT */
		const struct timer_mtimer_quirks *quirks = match->data;

		/* Set CLINT addresses */
		mt->mtimecmp_addr = addr[0] + ACLINT_DEFAULT_MTIMECMP_OFFSET;
		mt->mtimecmp_size = ACLINT_DEFAULT_MTIMECMP_SIZE;
		if (!quirks->without_mtime) {
			mt->mtime_addr = addr[0] + ACLINT_DEFAULT_MTIME_OFFSET;
			mt->mtime_size = size[0] - mt->mtimecmp_size;
			/* Adjust MTIMER address and size for CLINT device */
			mt->mtime_addr += quirks->mtime_offset;
			mt->mtime_size -= quirks->mtime_offset;
		} else {
			mt->mtime_addr = mt->mtime_size = 0;
		}
		mt->mtimecmp_addr += quirks->mtime_offset;
		/* Apply additional CLINT quirks */
		mt->has_64bit_mmio = quirks->has_64bit_mmio;
	} else { /* RISC-V ACLINT MTIMER */
		/* Set ACLINT MTIMER addresses */
		mt->mtime_addr = addr[0];
		mt->mtime_size = size[0];
		mt->mtimecmp_addr = addr[1];
		mt->mtimecmp_size = size[1];
	}

	/* Check if MTIMER device has shared MTIME address */
	if (mt->mtime_size) {
		mt->has_shared_mtime = false;
		for (i = 0; i < mtimer_count; i++) {
			if (mtimer[i].mtime_addr == mt->mtime_addr) {
				mt->has_shared_mtime = true;
				break;
			}
		}
	} else {
		/* Assume shared time CSR */
		mt->has_shared_mtime = true;
	}

	/* Initialize the MTIMER device */
	rc = aclint_mtimer_cold_init(mt, mt_reference);
	if (rc)
		return rc;

	/*
	 * Select first MTIMER device with no associated HARTs as our
	 * reference MTIMER device. This is only a temporary strategy
	 * of selecting reference MTIMER device. In future, we might
	 * define an optional DT property or some other mechanism to
	 * help us select the reference MTIMER device.
	 */
	if (!mt->hart_count && !mt_reference) {
		mt_reference = mt;
		/*
		 * Set reference for already propbed MTIMER devices
		 * with non-shared MTIME
		 */
		for (i = 0; i < mtimer_count; i++)
			if (!mtimer[i].has_shared_mtime)
				aclint_mtimer_set_reference(&mtimer[i], mt);
	}

	/* Explicitly sync-up MTIMER devices not associated with any HARTs */
	if (!mt->hart_count)
		aclint_mtimer_sync(mt);

	mtimer_count++;
	return 0;
}

static const struct timer_mtimer_quirks sifive_clint_quirks = {
	.mtime_offset	= CLINT_MTIMER_OFFSET,
	.has_64bit_mmio	= true,
};

static const struct timer_mtimer_quirks thead_clint_quirks = {
	.mtime_offset	= CLINT_MTIMER_OFFSET,
	.without_mtime  = true,
};

static const struct fdt_match timer_mtimer_match[] = {
	{ .compatible = "riscv,clint0", .data = &sifive_clint_quirks },
	{ .compatible = "sifive,clint0", .data = &sifive_clint_quirks },
	{ .compatible = "thead,c900-clint", .data = &thead_clint_quirks },
	{ .compatible = "riscv,aclint-mtimer" },
	{ },
};

struct fdt_timer fdt_timer_mtimer = {
	.match_table = timer_mtimer_match,
	.cold_init = timer_mtimer_cold_init,
	.warm_init = aclint_mtimer_warm_init,
	.exit = NULL,
};

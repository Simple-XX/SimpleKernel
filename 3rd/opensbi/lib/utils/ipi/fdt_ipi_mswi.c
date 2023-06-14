/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_error.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/ipi/fdt_ipi.h>
#include <sbi_utils/ipi/aclint_mswi.h>

#define MSWI_MAX_NR			16

static unsigned long mswi_count = 0;
static struct aclint_mswi_data mswi[MSWI_MAX_NR];

static int ipi_mswi_cold_init(void *fdt, int nodeoff,
			      const struct fdt_match *match)
{
	int rc;
	unsigned long offset;
	struct aclint_mswi_data *ms;

	if (MSWI_MAX_NR <= mswi_count)
		return SBI_ENOSPC;
	ms = &mswi[mswi_count];

	rc = fdt_parse_aclint_node(fdt, nodeoff, false,
				   &ms->addr, &ms->size, NULL, NULL,
				   &ms->first_hartid, &ms->hart_count);
	if (rc)
		return rc;

	if (match->data) {
		/* Adjust MSWI address and size for CLINT device */
		offset = *((unsigned long *)match->data);
		ms->addr += offset;
		if ((ms->size - offset) < ACLINT_MSWI_SIZE)
			return SBI_EINVAL;
		ms->size = ACLINT_MSWI_SIZE;
	}

	rc = aclint_mswi_cold_init(ms);
	if (rc)
		return rc;

	mswi_count++;
	return 0;
}

static const unsigned long clint_offset = CLINT_MSWI_OFFSET;

static const struct fdt_match ipi_mswi_match[] = {
	{ .compatible = "riscv,clint0", .data = &clint_offset },
	{ .compatible = "sifive,clint0", .data = &clint_offset },
	{ .compatible = "thead,c900-clint", .data = &clint_offset },
	{ .compatible = "riscv,aclint-mswi" },
	{ },
};

struct fdt_ipi fdt_ipi_mswi = {
	.match_table = ipi_mswi_match,
	.cold_init = ipi_mswi_cold_init,
	.warm_init = aclint_mswi_warm_init,
	.exit = NULL,
};

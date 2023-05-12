/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Andes Technology Corporation
 *
 * Authors:
 *   Yu Chien Peter Lin <peterlin@andestech.com>
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/timer/fdt_timer.h>
#include <sbi_utils/timer/andes_plmt.h>

extern struct plmt_data plmt;

static int fdt_plmt_cold_timer_init(void *fdt, int nodeoff,
				    const struct fdt_match *match)
{
	int rc;
	unsigned long plmt_base;

	rc = fdt_parse_plmt_node(fdt, nodeoff, &plmt_base, &plmt.size,
				 &plmt.hart_count);
	if (rc)
		return rc;

	plmt.time_val = (u64 *)plmt_base;
	plmt.time_cmp = (u64 *)(plmt_base + 0x8);

	rc = fdt_parse_timebase_frequency(fdt, &plmt.timer_freq);
	if (rc)
		return rc;

	rc = plmt_cold_timer_init(&plmt);
	if (rc)
		return rc;

	return 0;
}

static const struct fdt_match timer_plmt_match[] = {
	{ .compatible = "andestech,plmt0" },
	{},
};

struct fdt_timer fdt_timer_plmt = {
	.match_table = timer_plmt_match,
	.cold_init   = fdt_plmt_cold_timer_init,
	.warm_init   = plmt_warm_timer_init,
	.exit	     = NULL,
};

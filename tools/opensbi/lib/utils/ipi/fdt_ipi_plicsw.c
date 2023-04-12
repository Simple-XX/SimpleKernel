/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Andes Technology Corporation
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 *   Leo Yu-Chi Liang <ycliang@andestech.com>
 *   Yu Chien Peter Lin <peterlin@andestech.com>
 */

#include <sbi/riscv_io.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/ipi/fdt_ipi.h>
#include <sbi_utils/ipi/andes_plicsw.h>

extern struct plicsw_data plicsw;

int fdt_plicsw_cold_ipi_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	int rc;

	rc = fdt_parse_plicsw_node(fdt, nodeoff, &plicsw.addr, &plicsw.size,
				   &plicsw.hart_count);
	if (rc)
		return rc;

	rc = plicsw_cold_ipi_init(&plicsw);
	if (rc)
		return rc;

	return 0;
}

static const struct fdt_match ipi_plicsw_match[] = {
	{ .compatible = "andestech,plicsw" },
	{},
};

struct fdt_ipi fdt_ipi_plicsw = {
	.match_table = ipi_plicsw_match,
	.cold_init   = fdt_plicsw_cold_ipi_init,
	.warm_init   = plicsw_warm_ipi_init,
	.exit	     = NULL,
};

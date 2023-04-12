/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <libfdt.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_error.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>
#include <sbi_utils/irqchip/aplic.h>

#define APLIC_MAX_NR			16

static unsigned long aplic_count = 0;
static struct aplic_data aplic[APLIC_MAX_NR];

static int irqchip_aplic_warm_init(void)
{
	/* Nothing to do here. */
	return 0;
}

static int irqchip_aplic_cold_init(void *fdt, int nodeoff,
				  const struct fdt_match *match)
{
	int rc;
	struct aplic_data *pd;

	if (APLIC_MAX_NR <= aplic_count)
		return SBI_ENOSPC;
	pd = &aplic[aplic_count++];

	rc = fdt_parse_aplic_node(fdt, nodeoff, pd);
	if (rc)
		return rc;

	return aplic_cold_irqchip_init(pd);
}

static const struct fdt_match irqchip_aplic_match[] = {
	{ .compatible = "riscv,aplic" },
	{ },
};

struct fdt_irqchip fdt_irqchip_aplic = {
	.match_table = irqchip_aplic_match,
	.cold_init = irqchip_aplic_cold_init,
	.warm_init = irqchip_aplic_warm_init,
	.exit = NULL,
};

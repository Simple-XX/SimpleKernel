/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_error.h>
#include <sbi/sbi_scratch.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>

/* List of FDT irqchip drivers generated at compile time */
extern struct fdt_irqchip *fdt_irqchip_drivers[];
extern unsigned long fdt_irqchip_drivers_size;

#define FDT_IRQCHIP_MAX_DRIVERS	8

static struct fdt_irqchip *current_drivers[FDT_IRQCHIP_MAX_DRIVERS] = {0};
static int current_drivers_count;

void fdt_irqchip_exit(void)
{
	int i;

	for (i = 0; i < current_drivers_count; i++) {
		if (!current_drivers[i] || !current_drivers[i]->exit)
			continue;
		current_drivers[i]->exit();
	}
}

static int fdt_irqchip_warm_init(void)
{
	int i, rc;

	for (i = 0; i < current_drivers_count; i++) {
		if (!current_drivers[i] || !current_drivers[i]->warm_init)
			continue;
		rc = current_drivers[i]->warm_init();
		if (rc)
			return rc;
	}

	return 0;
}

static int fdt_irqchip_cold_init(void)
{
	bool drv_added;
	int pos, noff, rc;
	struct fdt_irqchip *drv;
	const struct fdt_match *match;
	void *fdt = fdt_get_address();

	for (pos = 0; pos < fdt_irqchip_drivers_size; pos++) {
		drv = fdt_irqchip_drivers[pos];

		noff = -1;
		drv_added = false;
		while ((noff = fdt_find_match(fdt, noff,
					drv->match_table, &match)) >= 0) {
			if (drv->cold_init) {
				rc = drv->cold_init(fdt, noff, match);
				if (rc == SBI_ENODEV)
					continue;
				if (rc)
					return rc;
			}

			if (drv_added)
				continue;

			current_drivers[current_drivers_count++] = drv;
			drv_added = true;
		}

		if (FDT_IRQCHIP_MAX_DRIVERS <= current_drivers_count)
			break;
	}

	return 0;
}

int fdt_irqchip_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = fdt_irqchip_cold_init();
		if (rc)
			return rc;
	}

	return fdt_irqchip_warm_init();
}

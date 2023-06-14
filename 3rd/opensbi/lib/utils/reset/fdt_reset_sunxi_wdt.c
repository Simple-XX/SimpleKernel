/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Samuel Holland <samuel@sholland.org>
 */

#include <libfdt.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_system.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/reset/fdt_reset.h>

#define WDT_KEY_VAL			0x16aa0000

#define WDT_SOFT_RST_REG		0x08
#define WDT_SOFT_RST_EN			BIT(0)

#define WDT_MODE_REG			0x18

static volatile char *sunxi_wdt_base;

static int sunxi_wdt_system_reset_check(u32 type, u32 reason)
{
	switch (type) {
	case SBI_SRST_RESET_TYPE_COLD_REBOOT:
	case SBI_SRST_RESET_TYPE_WARM_REBOOT:
		return 1;
	}

	return 0;
}

static void sunxi_wdt_system_reset(u32 type, u32 reason)
{
	/* Disable the watchdog. */
	writel_relaxed(WDT_KEY_VAL,
		       sunxi_wdt_base + WDT_MODE_REG);

	/* Trigger soft reset. */
	writel_relaxed(WDT_KEY_VAL | WDT_SOFT_RST_EN,
		       sunxi_wdt_base + WDT_SOFT_RST_REG);
}

static struct sbi_system_reset_device sunxi_wdt_reset = {
	.name = "sunxi-wdt-reset",
	.system_reset_check = sunxi_wdt_system_reset_check,
	.system_reset = sunxi_wdt_system_reset,
};

static int sunxi_wdt_reset_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	uint64_t reg_addr;
	int rc;

	rc = fdt_get_node_addr_size(fdt, nodeoff, 0, &reg_addr, NULL);
	if (rc < 0 || !reg_addr)
		return SBI_ENODEV;

	sunxi_wdt_base = (volatile char *)(unsigned long)reg_addr;

	sbi_system_reset_add_device(&sunxi_wdt_reset);

	return 0;
}

static const struct fdt_match sunxi_wdt_reset_match[] = {
	{ .compatible = "allwinner,sun20i-d1-wdt-reset" },
	{ },
};

struct fdt_reset fdt_reset_sunxi_wdt = {
	.match_table = sunxi_wdt_reset_match,
	.init = sunxi_wdt_reset_init,
};

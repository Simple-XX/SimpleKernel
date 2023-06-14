/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Andes Technology Corporation
 *
 * Authors:
 *   Yu Chien Peter Lin <peterlin@andestech.com>
 */

#include <libfdt.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_system.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/reset/fdt_reset.h>

#define ATCWDT200_WP_NUM 0x5aa5
#define WREN_REG 0x18
#define CTRL_REG 0x10
#define RST_TIME_OFF 8
#define RST_TIME_MSK (0x3 << RST_TIME_OFF)
#define RST_CLK_128 (0 << RST_TIME_OFF)
#define RST_CLK_256 (1 << RST_TIME_OFF)
#define RST_CLK_512 (2 << RST_TIME_OFF)
#define RST_CLK_1024 (3 << RST_TIME_OFF)
#define INT_TIME_OFF 4
#define INT_TIME_MSK (0xf << INT_TIME_OFF)
#define INT_CLK_64 (0 << INT_TIME_OFF)
#define INT_CLK_256 (1 << INT_TIME_OFF)
#define INT_CLK_1024 (2 << INT_TIME_OFF)
#define INT_CLK_2048 (3 << INT_TIME_OFF)
#define INT_CLK_4096 (4 << INT_TIME_OFF)
#define INT_CLK_8192 (5 << INT_TIME_OFF)
#define INT_CLK_16384 (6 << INT_TIME_OFF)
#define INT_CLK_32768 (7 << INT_TIME_OFF)
#define RST_EN (1 << 3)
#define INT_EN (1 << 2)
#define CLK_PCLK (1 << 1)
#define WDT_EN (1 << 0)

#define FLASH_BASE 0x80000000ULL
#define SMU_RESET_VEC_LO_OFF 0x50
#define SMU_RESET_VEC_HI_OFF 0x60
#define SMU_HARTn_RESET_VEC_LO(n) (SMU_RESET_VEC_LO_OFF + (n * 0x4))
#define SMU_HARTn_RESET_VEC_HI(n) (SMU_RESET_VEC_HI_OFF + (n * 0x4))

static volatile char *wdt_addr;
static volatile char *smu_addr;

static int ae350_system_reset_check(u32 type, u32 reason)
{
	switch (type) {
	case SBI_SRST_RESET_TYPE_COLD_REBOOT:
		return 1;
	case SBI_SRST_RESET_TYPE_SHUTDOWN:
	case SBI_SRST_RESET_TYPE_WARM_REBOOT:
	default:
		return 0;
	}
}

static void ae350_system_reset(u32 type, u32 reason)
{
	const struct sbi_platform *plat = sbi_platform_thishart_ptr();

	for (int i = 0; i < sbi_platform_hart_count(plat); i++) {
		writel(FLASH_BASE, smu_addr + SMU_HARTn_RESET_VEC_LO(i));
		writel(FLASH_BASE >> 32, smu_addr + SMU_HARTn_RESET_VEC_HI(i));
	}

	/* Program WDT control register  */
	writew(ATCWDT200_WP_NUM, wdt_addr + WREN_REG);
	writel(INT_CLK_32768 | INT_EN | RST_CLK_128 | RST_EN | WDT_EN,
	       wdt_addr + CTRL_REG);

	sbi_hart_hang();
}

static struct sbi_system_reset_device atcwdt200_reset = {
	.name		    = "atcwdt200",
	.system_reset_check = ae350_system_reset_check,
	.system_reset	    = ae350_system_reset,
};

static int atcwdt200_reset_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	uint64_t reg_addr;
	int rc;

	rc = fdt_get_node_addr_size(fdt, nodeoff, 0, &reg_addr, NULL);
	if (rc < 0 || !reg_addr)
		return SBI_ENODEV;

	wdt_addr = (volatile char *)(unsigned long)reg_addr;

	/*
	 * The reset device requires smu to program the reset
	 * vector for each hart.
	 */
	if (fdt_parse_compat_addr(fdt, &reg_addr, "andestech,atcsmu"))
		return SBI_ENODEV;

	smu_addr = (volatile char *)(unsigned long)reg_addr;

	sbi_system_reset_add_device(&atcwdt200_reset);

	return 0;
}

static const struct fdt_match atcwdt200_reset_match[] = {
	{ .compatible = "andestech,atcwdt200" },
	{},
};

struct fdt_reset fdt_reset_atcwdt200 = {
	.match_table = atcwdt200_reset_match,
	.init	     = atcwdt200_reset_init,
};

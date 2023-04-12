/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 SiFive
 * Copyright (c) 2021 YADRO
 *
 * Authors:
 *   David Abdurachmanov <david.abdurachmanov@sifive.com>
 *   Nikita Shubin <n.shubin@yadro.com>
 */

#include <platform_override.h>
#include <libfdt.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_console.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/reset/fdt_reset.h>
#include <sbi_utils/i2c/fdt_i2c.h>

#define DA9063_REG_PAGE_CON		0x00
#define DA9063_REG_CONTROL_A		0x0e
#define DA9063_REG_CONTROL_D		0x11
#define DA9063_REG_CONTROL_F		0x13
#define DA9063_REG_DEVICE_ID		0x81

#define DA9063_CONTROL_A_M_POWER1_EN	(1 << 6)
#define DA9063_CONTROL_A_M_POWER_EN	(1 << 5)
#define DA9063_CONTROL_A_STANDBY	(1 << 3)

#define DA9063_CONTROL_D_TWDSCALE_MASK	0x07

#define DA9063_CONTROL_F_WAKEUP	(1 << 2)
#define DA9063_CONTROL_F_SHUTDOWN	(1 << 1)

#define PMIC_CHIP_ID_DA9063		0x61

static struct {
	struct i2c_adapter *adapter;
	uint32_t reg;
} da9063;

static int da9063_system_reset_check(u32 type, u32 reason)
{
	switch (type) {
	case SBI_SRST_RESET_TYPE_SHUTDOWN:
		return 1;
	case SBI_SRST_RESET_TYPE_COLD_REBOOT:
	case SBI_SRST_RESET_TYPE_WARM_REBOOT:
		return 255;
	}

	return 0;
}

static inline int da9063_sanity_check(struct i2c_adapter *adap, uint32_t reg)
{
	uint8_t val;
	int rc = i2c_adapter_reg_write(adap, reg, DA9063_REG_PAGE_CON, 0x02);

	if (rc)
		return rc;

	/* check set page*/
	rc = i2c_adapter_reg_read(adap, reg, 0x0, &val);
	if (rc)
		return rc;

	if (val != 0x02)
		return SBI_ENODEV;

	/* read and check device id */
	rc = i2c_adapter_reg_read(adap, reg, DA9063_REG_DEVICE_ID, &val);
	if (rc)
		return rc;

	if (val != PMIC_CHIP_ID_DA9063)
		return SBI_ENODEV;

	return 0;
}

static inline int da9063_stop_watchdog(struct i2c_adapter *adap, uint32_t reg)
{
	uint8_t val;
	int rc = i2c_adapter_reg_write(adap, reg,
					DA9063_REG_PAGE_CON, 0x00);

	if (rc)
		return rc;

	rc = i2c_adapter_reg_read(adap, reg, DA9063_REG_CONTROL_D, &val);
	if (rc)
		return rc;

	if ((val & DA9063_CONTROL_D_TWDSCALE_MASK) == 0)
		return 0;

	val &= ~DA9063_CONTROL_D_TWDSCALE_MASK;

	return i2c_adapter_reg_write(adap, reg, DA9063_REG_CONTROL_D, val);
}

static inline int da9063_shutdown(struct i2c_adapter *adap, uint32_t reg)
{
	int rc = i2c_adapter_reg_write(adap, reg,
					DA9063_REG_PAGE_CON, 0x00);

	if (rc)
		return rc;

	return i2c_adapter_reg_write(adap, reg,
				     DA9063_REG_CONTROL_F,
				     DA9063_CONTROL_F_SHUTDOWN);
}

static inline int da9063_reset(struct i2c_adapter *adap, uint32_t reg)
{
	int rc = i2c_adapter_reg_write(adap, reg,
					DA9063_REG_PAGE_CON, 0x00);

	if (rc)
		return rc;

	rc = i2c_adapter_reg_write(adap, reg,
				   DA9063_REG_CONTROL_F,
				   DA9063_CONTROL_F_WAKEUP);
	if (rc)
		return rc;

	return i2c_adapter_reg_write(adap, reg,
				DA9063_REG_CONTROL_A,
				DA9063_CONTROL_A_M_POWER1_EN |
				DA9063_CONTROL_A_M_POWER_EN |
				DA9063_CONTROL_A_STANDBY);
}

static void da9063_system_reset(u32 type, u32 reason)
{
	struct i2c_adapter *adap = da9063.adapter;
	uint32_t reg = da9063.reg;
	int rc;

	if (adap) {
		/* sanity check */
		rc = da9063_sanity_check(adap, reg);
		if (rc) {
			sbi_printf("%s: chip is not da9063 PMIC\n", __func__);
			goto skip_reset;
		}

		switch (type) {
		case SBI_SRST_RESET_TYPE_SHUTDOWN:
			da9063_shutdown(adap, reg);
			break;
		case SBI_SRST_RESET_TYPE_COLD_REBOOT:
		case SBI_SRST_RESET_TYPE_WARM_REBOOT:
			da9063_stop_watchdog(adap, reg);
			da9063_reset(adap, reg);
			break;
		}
	}

skip_reset:
	sbi_hart_hang();
}

static struct sbi_system_reset_device da9063_reset_i2c = {
	.name = "da9063-reset",
	.system_reset_check = da9063_system_reset_check,
	.system_reset = da9063_system_reset
};

static int da9063_reset_init(void *fdt, int nodeoff,
			     const struct fdt_match *match)
{
	int rc, i2c_bus;
	struct i2c_adapter *adapter;
	uint64_t addr;

	/* we are dlg,da9063 node */
	rc = fdt_get_node_addr_size(fdt, nodeoff, 0, &addr, NULL);
	if (rc)
		return rc;

	da9063.reg = addr;

	/* find i2c bus parent node */
	i2c_bus = fdt_parent_offset(fdt, nodeoff);
	if (i2c_bus < 0)
		return i2c_bus;

	/* i2c adapter get */
	rc = fdt_i2c_adapter_get(fdt, i2c_bus, &adapter);
	if (rc)
		return rc;

	da9063.adapter = adapter;

	sbi_system_reset_add_device(&da9063_reset_i2c);

	return 0;
}

static const struct fdt_match da9063_reset_match[] = {
	{ .compatible = "dlg,da9063", .data = (void *)TRUE },
	{ },
};

struct fdt_reset fdt_reset_da9063 = {
	.match_table = da9063_reset_match,
	.init = da9063_reset_init,
};

static u64 sifive_fu740_tlbr_flush_limit(const struct fdt_match *match)
{
	/*
	 * Needed to address CIP-1200 errata on SiFive FU740
	 * Title: Instruction TLB can fail to respect a non-global SFENCE
	 * Workaround: Flush the TLB using SFENCE.VMA x0, x0
	 * See Errata_FU740-C000_20210205 from
	 * https://www.sifive.com/boards/hifive-unmatched
	 */
	return 0;
}

static int sifive_fu740_final_init(bool cold_boot,
				   const struct fdt_match *match)
{
	int rc;
	void *fdt = fdt_get_address();

	if (cold_boot) {
		rc = fdt_reset_driver_init(fdt, &fdt_reset_da9063);
		if (rc)
			sbi_printf("%s: failed to find da9063 for reset\n",
				   __func__);
	}

	return 0;
}

static const struct fdt_match sifive_fu740_match[] = {
	{ .compatible = "sifive,fu740" },
	{ .compatible = "sifive,fu740-c000" },
	{ .compatible = "sifive,hifive-unmatched-a00" },
	{ },
};

const struct platform_override sifive_fu740 = {
	.match_table = sifive_fu740_match,
	.tlbr_flush_limit = sifive_fu740_tlbr_flush_limit,
	.final_init = sifive_fu740_final_init,
};

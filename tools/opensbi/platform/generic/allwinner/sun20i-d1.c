/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Samuel Holland <samuel@sholland.org>
 */

#include <platform_override.h>
#include <thead_c9xx.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hsm.h>
#include <sbi/sbi_pmu.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/irqchip/fdt_irqchip_plic.h>

#define SUN20I_D1_CCU_BASE		((void *)0x02001000)
#define SUN20I_D1_RISCV_CFG_BASE	((void *)0x06010000)
#define SUN20I_D1_PPU_BASE		((void *)0x07001000)
#define SUN20I_D1_PRCM_BASE		((void *)0x07010000)

/*
 * CCU
 */

#define CCU_BGR_ENABLE			(BIT(16) | BIT(0))

#define RISCV_CFG_BGR_REG		0xd0c
#define PPU_BGR_REG			0x1ac

/*
 * CSRs
 */

#define CSR_MXSTATUS			0x7c0
#define CSR_MHCR			0x7c1
#define CSR_MCOR			0x7c2
#define CSR_MHINT			0x7c5

static unsigned long csr_mxstatus;
static unsigned long csr_mhcr;
static unsigned long csr_mhint;

static void sun20i_d1_csr_save(void)
{
	/* Save custom CSRs. */
	csr_mxstatus	= csr_read(CSR_MXSTATUS);
	csr_mhcr	= csr_read(CSR_MHCR);
	csr_mhint	= csr_read(CSR_MHINT);

	/* Flush and disable caches. */
	csr_write(CSR_MCOR, 0x22);
	csr_write(CSR_MHCR, 0x0);
}

static void sun20i_d1_csr_restore(void)
{
	/* Invalidate caches and the branch predictor. */
	csr_write(CSR_MCOR, 0x70013);

	/* Restore custom CSRs, including the cache state. */
	csr_write(CSR_MXSTATUS,	csr_mxstatus);
	csr_write(CSR_MHCR,	csr_mhcr);
	csr_write(CSR_MHINT,	csr_mhint);
}

/*
 * PLIC
 */

#define PLIC_SOURCES			176
#define PLIC_IE_WORDS			((PLIC_SOURCES + 31) / 32)

static u8 plic_priority[PLIC_SOURCES];
static u32 plic_sie[PLIC_IE_WORDS];
static u32 plic_threshold;

static void sun20i_d1_plic_save(void)
{
	fdt_plic_context_save(true, plic_sie, &plic_threshold, PLIC_IE_WORDS);
	fdt_plic_priority_save(plic_priority, PLIC_SOURCES);
}

static void sun20i_d1_plic_restore(void)
{
	thead_plic_restore();
	fdt_plic_priority_restore(plic_priority, PLIC_SOURCES);
	fdt_plic_context_restore(true, plic_sie, plic_threshold,
				 PLIC_IE_WORDS);
}

/*
 * PPU
 */

#define PPU_PD_ACTIVE_CTRL		0x2c

static void sun20i_d1_ppu_save(void)
{
	/* Enable MMIO access. Do not assume S-mode leaves the clock enabled. */
	writel_relaxed(CCU_BGR_ENABLE, SUN20I_D1_PRCM_BASE + PPU_BGR_REG);

	/* Activate automatic power-down during the next WFI. */
	writel_relaxed(1, SUN20I_D1_PPU_BASE + PPU_PD_ACTIVE_CTRL);
}

static void sun20i_d1_ppu_restore(void)
{
	/* Disable automatic power-down. */
	writel_relaxed(0, SUN20I_D1_PPU_BASE + PPU_PD_ACTIVE_CTRL);
}

/*
 * RISCV_CFG
 */

#define RESET_ENTRY_LO_REG		0x0004
#define RESET_ENTRY_HI_REG		0x0008
#define WAKEUP_EN_REG			0x0020
#define WAKEUP_MASK_REG(i)		(0x0024 + 4 * (i))

static void sun20i_d1_riscv_cfg_save(void)
{
	/* Enable MMIO access. Do not assume S-mode leaves the clock enabled. */
	writel_relaxed(CCU_BGR_ENABLE, SUN20I_D1_CCU_BASE + RISCV_CFG_BGR_REG);

	/*
	 * Copy the SIE bits to the wakeup registers. D1 has 160 "real"
	 * interrupt sources, numbered 16-175. These are the ones that map to
	 * the wakeup mask registers (the offset is for GIC compatibility). So
	 * copying SIE to the wakeup mask needs some bit manipulation.
	 */
	for (int i = 0; i < PLIC_IE_WORDS - 1; i++)
		writel_relaxed(plic_sie[i] >> 16 | plic_sie[i + 1] << 16,
			       SUN20I_D1_RISCV_CFG_BASE + WAKEUP_MASK_REG(i));

	/* Enable PPU wakeup for interrupts. */
	writel_relaxed(1, SUN20I_D1_RISCV_CFG_BASE + WAKEUP_EN_REG);
}

static void sun20i_d1_riscv_cfg_restore(void)
{
	/* Disable PPU wakeup for interrupts. */
	writel_relaxed(0, SUN20I_D1_RISCV_CFG_BASE + WAKEUP_EN_REG);
}

static void sun20i_d1_riscv_cfg_init(void)
{
	u64 entry = sbi_hartid_to_scratch(0)->warmboot_addr;

	/* Enable MMIO access. */
	writel_relaxed(CCU_BGR_ENABLE, SUN20I_D1_CCU_BASE + RISCV_CFG_BGR_REG);

	/* Program the reset entry address. */
	writel_relaxed(entry, SUN20I_D1_RISCV_CFG_BASE + RESET_ENTRY_LO_REG);
	writel_relaxed(entry >> 32, SUN20I_D1_RISCV_CFG_BASE + RESET_ENTRY_HI_REG);
}

static int sun20i_d1_hart_suspend(u32 suspend_type)
{
	/* Use the generic code for retentive suspend. */
	if (!(suspend_type & SBI_HSM_SUSP_NON_RET_BIT))
		return SBI_ENOTSUPP;

	sun20i_d1_plic_save();
	sun20i_d1_ppu_save();
	sun20i_d1_riscv_cfg_save();
	sun20i_d1_csr_save();

	/*
	 * If no interrupt is pending, this will power down the CPU power
	 * domain. Otherwise, this will fall through, and the generic HSM
	 * code will jump to the resume address.
	 */
	wfi();

	return 0;
}

static void sun20i_d1_hart_resume(void)
{
	sun20i_d1_csr_restore();
	sun20i_d1_riscv_cfg_restore();
	sun20i_d1_ppu_restore();
	sun20i_d1_plic_restore();
}

static const struct sbi_hsm_device sun20i_d1_ppu = {
	.name		= "sun20i-d1-ppu",
	.hart_suspend	= sun20i_d1_hart_suspend,
	.hart_resume	= sun20i_d1_hart_resume,
};

static int sun20i_d1_final_init(bool cold_boot, const struct fdt_match *match)
{
	if (cold_boot) {
		sun20i_d1_riscv_cfg_init();
		sbi_hsm_set_device(&sun20i_d1_ppu);
	}

	return 0;
}

static void thead_c9xx_pmu_ctr_enable_irq(uint32_t ctr_idx)
{
	unsigned long mip_val;

	if (ctr_idx >= SBI_PMU_HW_CTR_MAX)
		return;

	mip_val = csr_read(CSR_MIP);
	/**
	 * Clear out the OF bit so that next interrupt can be enabled.
	 * This should be done only when the corresponding overflow interrupt
	 * bit is cleared. That indicates that software has already handled the
	 * previous interrupts or the hardware yet to set an overflow interrupt.
	 * Otherwise, there will be race conditions where we may clear the bit
	 * the software is yet to handle the interrupt.
	 */
	if (!(mip_val & THEAD_C9XX_MIP_MOIP))
		csr_clear(THEAD_C9XX_CSR_MCOUNTEROF, BIT(ctr_idx));

	/**
	 * SSCOFPMF uses the OF bit for enabling/disabling the interrupt,
	 * while the C9XX has designated enable bits.
	 * So enable per-counter interrupt on C9xx here.
	 */
	csr_set(THEAD_C9XX_CSR_MCOUNTERINTEN, BIT(ctr_idx));
}

static void thead_c9xx_pmu_ctr_disable_irq(uint32_t ctr_idx)
{
	csr_clear(THEAD_C9XX_CSR_MCOUNTERINTEN, BIT(ctr_idx));
}

static int thead_c9xx_pmu_irq_bit(void)
{
	return THEAD_C9XX_MIP_MOIP;
}

const struct sbi_pmu_device thead_c9xx_pmu_device = {
	.hw_counter_enable_irq = thead_c9xx_pmu_ctr_enable_irq,
	.hw_counter_disable_irq = thead_c9xx_pmu_ctr_disable_irq,
	.hw_counter_irq_bit = thead_c9xx_pmu_irq_bit,
};

static int sun20i_d1_extensions_init(const struct fdt_match *match,
				     struct sbi_hart_features *hfeatures)
{
	sbi_pmu_set_device(&thead_c9xx_pmu_device);

	/* auto-detection doesn't work on t-head c9xx cores */
	hfeatures->mhpm_count = 29;
	hfeatures->mhpm_bits = 64;

	return 0;
}

static const struct fdt_match sun20i_d1_match[] = {
	{ .compatible = "allwinner,sun20i-d1" },
	{ },
};

const struct platform_override sun20i_d1 = {
	.match_table	= sun20i_d1_match,
	.final_init	= sun20i_d1_final_init,
	.extensions_init = sun20i_d1_extensions_init,
};

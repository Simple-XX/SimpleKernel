/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <apatel@ventanamicro.com>
 */

#include <sbi/sbi_irqchip.h>
#include <sbi/sbi_platform.h>

static int default_irqfn(struct sbi_trap_regs *regs)
{
	return SBI_ENODEV;
}

static int (*ext_irqfn)(struct sbi_trap_regs *regs) = default_irqfn;

void sbi_irqchip_set_irqfn(int (*fn)(struct sbi_trap_regs *regs))
{
	if (fn)
		ext_irqfn = fn;
}

int sbi_irqchip_process(struct sbi_trap_regs *regs)
{
	return ext_irqfn(regs);
}

int sbi_irqchip_init(struct sbi_scratch *scratch, bool cold_boot)
{
	int rc;
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	rc = sbi_platform_irqchip_init(plat, cold_boot);
	if (rc)
		return rc;

	if (ext_irqfn != default_irqfn)
		csr_set(CSR_MIE, MIP_MEIP);

	return 0;
}

void sbi_irqchip_exit(struct sbi_scratch *scratch)
{
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	if (ext_irqfn != default_irqfn)
		csr_clear(CSR_MIE, MIP_MEIP);

	sbi_platform_irqchip_exit(plat);
}

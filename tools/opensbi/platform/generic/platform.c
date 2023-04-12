/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <libfdt.h>
#include <platform_override.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_string.h>
#include <sbi_utils/fdt/fdt_domain.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_pmu.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>
#include <sbi_utils/irqchip/imsic.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/timer/fdt_timer.h>
#include <sbi_utils/ipi/fdt_ipi.h>
#include <sbi_utils/reset/fdt_reset.h>
#include <sbi_utils/serial/semihosting.h>

/* List of platform override modules generated at compile time */
extern const struct platform_override *platform_override_modules[];
extern unsigned long platform_override_modules_size;

static const struct platform_override *generic_plat = NULL;
static const struct fdt_match *generic_plat_match = NULL;

static void fw_platform_lookup_special(void *fdt, int root_offset)
{
	const struct platform_override *plat;
	const struct fdt_match *match;
	int pos;

	for (pos = 0; pos < platform_override_modules_size; pos++) {
		plat = platform_override_modules[pos];
		if (!plat->match_table)
			continue;

		match = fdt_match_node(fdt, root_offset, plat->match_table);
		if (!match)
			continue;

		generic_plat = plat;
		generic_plat_match = match;
		break;
	}
}

extern struct sbi_platform platform;
static bool platform_has_mlevel_imsic = false;
static u32 generic_hart_index2id[SBI_HARTMASK_MAX_BITS] = { 0 };

/*
 * The fw_platform_init() function is called very early on the boot HART
 * OpenSBI reference firmwares so that platform specific code get chance
 * to update "platform" instance before it is used.
 *
 * The arguments passed to fw_platform_init() function are boot time state
 * of A0 to A4 register. The "arg0" will be boot HART id and "arg1" will
 * be address of FDT passed by previous booting stage.
 *
 * The return value of fw_platform_init() function is the FDT location. If
 * FDT is unchanged (or FDT is modified in-place) then fw_platform_init()
 * can always return the original FDT location (i.e. 'arg1') unmodified.
 */
unsigned long fw_platform_init(unsigned long arg0, unsigned long arg1,
				unsigned long arg2, unsigned long arg3,
				unsigned long arg4)
{
	const char *model;
	void *fdt = (void *)arg1;
	u32 hartid, hart_count = 0;
	int rc, root_offset, cpus_offset, cpu_offset, len;

	root_offset = fdt_path_offset(fdt, "/");
	if (root_offset < 0)
		goto fail;

	fw_platform_lookup_special(fdt, root_offset);

	model = fdt_getprop(fdt, root_offset, "model", &len);
	if (model)
		sbi_strncpy(platform.name, model, sizeof(platform.name) - 1);

	if (generic_plat && generic_plat->features)
		platform.features = generic_plat->features(generic_plat_match);

	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		goto fail;

	fdt_for_each_subnode(cpu_offset, fdt, cpus_offset) {
		rc = fdt_parse_hart_id(fdt, cpu_offset, &hartid);
		if (rc)
			continue;

		if (SBI_HARTMASK_MAX_BITS <= hartid)
			continue;

		if (!fdt_node_is_enabled(fdt, cpu_offset))
			continue;

		generic_hart_index2id[hart_count++] = hartid;
	}

	platform.hart_count = hart_count;

	platform_has_mlevel_imsic = fdt_check_imsic_mlevel(fdt);

	/* Return original FDT pointer */
	return arg1;

fail:
	while (1)
		wfi();
}

static int generic_nascent_init(void)
{
	if (platform_has_mlevel_imsic)
		imsic_local_irqchip_init();
	return 0;
}

static int generic_early_init(bool cold_boot)
{
	if (!generic_plat || !generic_plat->early_init)
		return 0;

	return generic_plat->early_init(cold_boot, generic_plat_match);
}

static int generic_final_init(bool cold_boot)
{
	void *fdt;
	int rc;

	if (cold_boot)
		fdt_reset_init();

	if (generic_plat && generic_plat->final_init) {
		rc = generic_plat->final_init(cold_boot, generic_plat_match);
		if (rc)
			return rc;
	}

	if (!cold_boot)
		return 0;

	fdt = fdt_get_address();

	fdt_cpu_fixup(fdt);
	fdt_fixups(fdt);
	fdt_domain_fixup(fdt);

	if (generic_plat && generic_plat->fdt_fixup) {
		rc = generic_plat->fdt_fixup(fdt, generic_plat_match);
		if (rc)
			return rc;
	}

	return 0;
}

static int generic_vendor_ext_check(long extid)
{
	if (generic_plat && generic_plat->vendor_ext_check)
		return generic_plat->vendor_ext_check(extid,
						      generic_plat_match);

	return 0;
}

static int generic_vendor_ext_provider(long extid, long funcid,
				       const struct sbi_trap_regs *regs,
				       unsigned long *out_value,
				       struct sbi_trap_info *out_trap)
{
	if (generic_plat && generic_plat->vendor_ext_provider) {
		return generic_plat->vendor_ext_provider(extid, funcid, regs,
							 out_value, out_trap,
							 generic_plat_match);
	}

	return SBI_ENOTSUPP;
}

static void generic_early_exit(void)
{
	if (generic_plat && generic_plat->early_exit)
		generic_plat->early_exit(generic_plat_match);
}

static void generic_final_exit(void)
{
	if (generic_plat && generic_plat->final_exit)
		generic_plat->final_exit(generic_plat_match);
}

static int generic_extensions_init(struct sbi_hart_features *hfeatures)
{
	if (generic_plat && generic_plat->extensions_init)
		return generic_plat->extensions_init(generic_plat_match,
						     hfeatures);

	return 0;
}

static int generic_domains_init(void)
{
	return fdt_domains_populate(fdt_get_address());
}

static u64 generic_tlbr_flush_limit(void)
{
	if (generic_plat && generic_plat->tlbr_flush_limit)
		return generic_plat->tlbr_flush_limit(generic_plat_match);
	return SBI_PLATFORM_TLB_RANGE_FLUSH_LIMIT_DEFAULT;
}

static int generic_pmu_init(void)
{
	return fdt_pmu_setup(fdt_get_address());
}

static uint64_t generic_pmu_xlate_to_mhpmevent(uint32_t event_idx,
					       uint64_t data)
{
	uint64_t evt_val = 0;

	/* data is valid only for raw events and is equal to event selector */
	if (event_idx == SBI_PMU_EVENT_RAW_IDX)
		evt_val = data;
	else {
		/**
		 * Generic platform follows the SBI specification recommendation
		 * i.e. zero extended event_idx is used as mhpmevent value for
		 * hardware general/cache events if platform does't define one.
		 */
		evt_val = fdt_pmu_get_select_value(event_idx);
		if (!evt_val)
			evt_val = (uint64_t)event_idx;
	}

	return evt_val;
}

static int generic_console_init(void)
{
	if (semihosting_enabled())
		return semihosting_init();
	else
		return fdt_serial_init();
}

const struct sbi_platform_operations platform_ops = {
	.nascent_init		= generic_nascent_init,
	.early_init		= generic_early_init,
	.final_init		= generic_final_init,
	.early_exit		= generic_early_exit,
	.final_exit		= generic_final_exit,
	.extensions_init	= generic_extensions_init,
	.domains_init		= generic_domains_init,
	.console_init		= generic_console_init,
	.irqchip_init		= fdt_irqchip_init,
	.irqchip_exit		= fdt_irqchip_exit,
	.ipi_init		= fdt_ipi_init,
	.ipi_exit		= fdt_ipi_exit,
	.pmu_init		= generic_pmu_init,
	.pmu_xlate_to_mhpmevent = generic_pmu_xlate_to_mhpmevent,
	.get_tlbr_flush_limit	= generic_tlbr_flush_limit,
	.timer_init		= fdt_timer_init,
	.timer_exit		= fdt_timer_exit,
	.vendor_ext_check	= generic_vendor_ext_check,
	.vendor_ext_provider	= generic_vendor_ext_provider,
};

struct sbi_platform platform = {
	.opensbi_version	= OPENSBI_VERSION,
	.platform_version	=
		SBI_PLATFORM_VERSION(CONFIG_PLATFORM_GENERIC_MAJOR_VER,
				     CONFIG_PLATFORM_GENERIC_MINOR_VER),
	.name			= CONFIG_PLATFORM_GENERIC_NAME,
	.features		= SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count		= SBI_HARTMASK_MAX_BITS,
	.hart_index2id		= generic_hart_index2id,
	.hart_stack_size	= SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.platform_ops_addr	= (unsigned long)&platform_ops
};

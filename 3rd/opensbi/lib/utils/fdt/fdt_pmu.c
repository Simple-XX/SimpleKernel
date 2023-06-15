// SPDX-License-Identifier: BSD-2-Clause
/*
 * fdt_pmu.c - Flat Device Tree PMU helper routines
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *	Atish Patra <atish.patra@wdc.com>
 */

#include <libfdt.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_pmu.h>
#include <sbi_utils/fdt/fdt_helper.h>

#define FDT_PMU_HW_EVENT_MAX (SBI_PMU_HW_EVENT_MAX * 2)

struct fdt_pmu_hw_event_select {
	uint32_t eidx;
	uint64_t select;
};

static struct fdt_pmu_hw_event_select fdt_pmu_evt_select[FDT_PMU_HW_EVENT_MAX] = {0};
static uint32_t hw_event_count;

uint64_t fdt_pmu_get_select_value(uint32_t event_idx)
{
	int i;
	struct fdt_pmu_hw_event_select *event;

	for (i = 0; i < SBI_PMU_HW_EVENT_MAX; i++) {
		event = &fdt_pmu_evt_select[i];
		if (event->eidx == event_idx)
			return event->select;
	}

	return 0;
}

int fdt_pmu_fixup(void *fdt)
{
	int pmu_offset;
	struct sbi_scratch *scratch = sbi_scratch_thishart_ptr();

	if (!fdt)
		return SBI_EINVAL;

	pmu_offset = fdt_node_offset_by_compatible(fdt, -1, "riscv,pmu");
	if (pmu_offset < 0)
		return SBI_EFAIL;

	fdt_delprop(fdt, pmu_offset, "riscv,event-to-mhpmcounters");
	fdt_delprop(fdt, pmu_offset, "riscv,event-to-mhpmevent");
	fdt_delprop(fdt, pmu_offset, "riscv,raw-event-to-mhpmcounters");
	if (!sbi_hart_has_extension(scratch, SBI_HART_EXT_SSCOFPMF))
		fdt_delprop(fdt, pmu_offset, "interrupts-extended");

	return 0;
}

int fdt_pmu_setup(void *fdt)
{
	int i, pmu_offset, len, result;
	const u32 *event_val;
	const u32 *event_ctr_map;
	struct fdt_pmu_hw_event_select *event;
	uint64_t raw_selector, select_mask;
	u32 event_idx_start, event_idx_end, ctr_map;

	if (!fdt)
		return SBI_EINVAL;

	pmu_offset = fdt_node_offset_by_compatible(fdt, -1, "riscv,pmu");
	if (pmu_offset < 0)
		return SBI_EFAIL;

	event_ctr_map = fdt_getprop(fdt, pmu_offset,
				    "riscv,event-to-mhpmcounters", &len);
	if (event_ctr_map && len >= 8) {
		len = len / (sizeof(u32) * 3);
		for (i = 0; i < len; i++) {
			event_idx_start = fdt32_to_cpu(event_ctr_map[3 * i]);
			event_idx_end = fdt32_to_cpu(event_ctr_map[3 * i + 1]);
			ctr_map = fdt32_to_cpu(event_ctr_map[3 * i + 2]);
			result = sbi_pmu_add_hw_event_counter_map(
				event_idx_start, event_idx_end, ctr_map);
			if (result)
				return result;
		}
	}

	event_val = fdt_getprop(fdt, pmu_offset,
				"riscv,event-to-mhpmevent", &len);
	if (event_val && len >= 8) {
		len = len / (sizeof(u32) * 3);
		for (i = 0; i < len; i++) {
			event = &fdt_pmu_evt_select[hw_event_count];
			event->eidx = fdt32_to_cpu(event_val[3 * i]);
			event->select = fdt32_to_cpu(event_val[3 * i + 1]);
			event->select = (event->select << 32) |
					fdt32_to_cpu(event_val[3 * i + 2]);
			hw_event_count++;
		}
	}

	event_val = fdt_getprop(fdt, pmu_offset,
				"riscv,raw-event-to-mhpmcounters", &len);
	if (event_val && len >= 20) {
		len = len / (sizeof(u32) * 5);
		for (i = 0; i < len; i++) {
			raw_selector = fdt32_to_cpu(event_val[5 * i]);
			raw_selector = (raw_selector << 32) |
					fdt32_to_cpu(event_val[5 * i + 1]);
			select_mask = fdt32_to_cpu(event_val[5 * i + 2]);
			select_mask = (select_mask  << 32) |
					fdt32_to_cpu(event_val[5 * i + 3]);
			ctr_map = fdt32_to_cpu(event_val[5 * i + 4]);
			result = sbi_pmu_add_raw_event_counter_map(
					raw_selector, select_mask, ctr_map);
			if (result)
				return result;
		}
	}

	return 0;
}

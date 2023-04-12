/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *   Nick Kossifidis <mick@ics.forth.gr>
 */

#include <sbi/riscv_asm.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_hsm.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_init.h>

static SBI_LIST_HEAD(reset_devices_list);

const struct sbi_system_reset_device *sbi_system_reset_get_device(
					u32 reset_type, u32 reset_reason)
{
	struct sbi_system_reset_device *reset_dev = NULL;
	struct sbi_dlist *pos;
	/** lowest priority - any non zero is our candidate */
	int priority = 0;

	/* Check each reset device registered for supported reset type */
	sbi_list_for_each(pos, &(reset_devices_list)) {
		struct sbi_system_reset_device *dev =
			to_system_reset_device(pos);
		if (dev->system_reset_check) {
			int status = dev->system_reset_check(reset_type,
							     reset_reason);
			/** reset_type not supported */
			if (status == 0)
				continue;

			if (status > priority) {
				reset_dev = dev;
				priority = status;
			}
		}
	}

	return reset_dev;
}

void sbi_system_reset_add_device(struct sbi_system_reset_device *dev)
{
	if (!dev || !dev->system_reset_check)
		return;

	sbi_list_add(&(dev->node), &(reset_devices_list));
}

bool sbi_system_reset_supported(u32 reset_type, u32 reset_reason)
{
	return !!sbi_system_reset_get_device(reset_type, reset_reason);
}

void __noreturn sbi_system_reset(u32 reset_type, u32 reset_reason)
{
	ulong hbase = 0, hmask;
	u32 cur_hartid = current_hartid();
	struct sbi_domain *dom = sbi_domain_thishart_ptr();
	struct sbi_scratch *scratch = sbi_scratch_thishart_ptr();

	/* Send HALT IPI to every hart other than the current hart */
	while (!sbi_hsm_hart_interruptible_mask(dom, hbase, &hmask)) {
		if (hbase <= cur_hartid)
			hmask &= ~(1UL << (cur_hartid - hbase));
		if (hmask)
			sbi_ipi_send_halt(hmask, hbase);
		hbase += BITS_PER_LONG;
	}

	/* Stop current HART */
	sbi_hsm_hart_stop(scratch, FALSE);

	/* Platform specific reset if domain allowed system reset */
	if (dom->system_reset_allowed) {
		const struct sbi_system_reset_device *dev =
			sbi_system_reset_get_device(reset_type, reset_reason);
		if (dev)
			dev->system_reset(reset_type, reset_reason);
	}

	/* If platform specific reset did not work then do sbi_exit() */
	sbi_exit(scratch);
}

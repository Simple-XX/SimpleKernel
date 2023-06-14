/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *   Atish Patra <atish.patra@wdc.com>
 */

#include <sbi/sbi_error.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_system.h>

static int sbi_ecall_srst_handler(unsigned long extid, unsigned long funcid,
				  const struct sbi_trap_regs *regs,
				  unsigned long *out_val,
				  struct sbi_trap_info *out_trap)
{
	if (funcid == SBI_EXT_SRST_RESET) {
		if ((((u32)-1U) <= ((u64)regs->a0)) ||
		    (((u32)-1U) <= ((u64)regs->a1)))
			return SBI_EINVAL;

		switch (regs->a0) {
		case SBI_SRST_RESET_TYPE_SHUTDOWN:
		case SBI_SRST_RESET_TYPE_COLD_REBOOT:
		case SBI_SRST_RESET_TYPE_WARM_REBOOT:
			break;
		default:
			return SBI_EINVAL;
		}

		switch (regs->a1) {
		case SBI_SRST_RESET_REASON_NONE:
		case SBI_SRST_RESET_REASON_SYSFAIL:
			break;
		default:
			return SBI_EINVAL;
		}

		if (sbi_system_reset_supported(regs->a0, regs->a1))
			sbi_system_reset(regs->a0, regs->a1);
	}

	return SBI_ENOTSUPP;
}

static int sbi_ecall_srst_probe(unsigned long extid, unsigned long *out_val)
{
	u32 type, count = 0;

	/*
	 * At least one standard reset types should be supported by
	 * the platform for SBI SRST extension to be usable.
	 */

	for (type = 0; type <= SBI_SRST_RESET_TYPE_LAST; type++) {
		if (sbi_system_reset_supported(type,
					SBI_SRST_RESET_REASON_NONE))
			count++;
	}

	*out_val = (count) ? 1 : 0;
	return 0;
}

struct sbi_ecall_extension ecall_srst = {
	.extid_start = SBI_EXT_SRST,
	.extid_end = SBI_EXT_SRST,
	.handle = sbi_ecall_srst_handler,
	.probe = sbi_ecall_srst_probe,
};

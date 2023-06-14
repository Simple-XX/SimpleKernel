/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_version.h>
#include <sbi/sbi_pmu.h>
#include <sbi/sbi_scratch.h>
#include <sbi/riscv_asm.h>

static int sbi_ecall_pmu_handler(unsigned long extid, unsigned long funcid,
				 const struct sbi_trap_regs *regs,
				 unsigned long *out_val,
				 struct sbi_trap_info *out_trap)
{
	int ret = 0;
	uint64_t temp;

	switch (funcid) {
	case SBI_EXT_PMU_NUM_COUNTERS:
		ret = sbi_pmu_num_ctr();
		if (ret >= 0) {
			*out_val = ret;
			ret = 0;
		}
		break;
	case SBI_EXT_PMU_COUNTER_GET_INFO:
		ret = sbi_pmu_ctr_get_info(regs->a0, out_val);
		break;
	case SBI_EXT_PMU_COUNTER_CFG_MATCH:
#if __riscv_xlen == 32
		temp = ((uint64_t)regs->a5 << 32) | regs->a4;
#else
		temp = regs->a4;
#endif
		ret = sbi_pmu_ctr_cfg_match(regs->a0, regs->a1, regs->a2,
					    regs->a3, temp);
		if (ret >= 0) {
			*out_val = ret;
			ret = 0;
		}

		break;
	case SBI_EXT_PMU_COUNTER_FW_READ:
		ret = sbi_pmu_ctr_fw_read(regs->a0, &temp);
		*out_val = temp;
		break;
	case SBI_EXT_PMU_COUNTER_START:

#if __riscv_xlen == 32
		temp = ((uint64_t)regs->a4 << 32) | regs->a3;
#else
		temp = regs->a3;
#endif
		ret = sbi_pmu_ctr_start(regs->a0, regs->a1, regs->a2, temp);
		break;
	case SBI_EXT_PMU_COUNTER_STOP:
		ret = sbi_pmu_ctr_stop(regs->a0, regs->a1, regs->a2);
		break;
	default:
		ret = SBI_ENOTSUPP;
	};

	return ret;
}

static int sbi_ecall_pmu_probe(unsigned long extid, unsigned long *out_val)
{
	/* PMU extension is always enabled */
	*out_val = 1;
	return 0;
}

struct sbi_ecall_extension ecall_pmu = {
	.extid_start = SBI_EXT_PMU,
	.extid_end = SBI_EXT_PMU,
	.handle = sbi_ecall_pmu_handler,
	.probe = sbi_ecall_pmu_probe,
};

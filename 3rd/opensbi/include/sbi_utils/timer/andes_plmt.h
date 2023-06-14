/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Andes Technology Corporation
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 *   Yu Chien Peter Lin <peterlin@andestech.com>
 */

#ifndef __TIMER_ANDES_PLMT_H__
#define __TIMER_ANDES_PLMT_H__

#define DEFAULT_AE350_PLMT_FREQ 60000000
#define PLMT_REGION_ALIGN 0x1000

struct plmt_data {
	u32 hart_count;
	unsigned long size;
	unsigned long timer_freq;
	volatile u64 *time_val;
	volatile u64 *time_cmp;
};

int plmt_cold_timer_init(struct plmt_data *plmt);
int plmt_warm_timer_init(void);

#endif /* __TIMER_ANDES_PLMT_H__ */

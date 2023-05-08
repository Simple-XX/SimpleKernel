/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Andes Technology Corporation
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 *   Leo Yu-Chi Liang <ycliang@andestech.com>
 *   Yu Chien Peter Lin <peterlin@andestech.com>
 */

#ifndef _IPI_ANDES_PLICSW_H_
#define _IPI_ANDES_PLICSW_H_

#define PLICSW_PRIORITY_BASE 0x4

#define PLICSW_PENDING_BASE 0x1000
#define PLICSW_PENDING_STRIDE 0x8

#define PLICSW_ENABLE_BASE 0x2000
#define PLICSW_ENABLE_STRIDE 0x80

#define PLICSW_CONTEXT_BASE 0x200000
#define PLICSW_CONTEXT_STRIDE 0x1000
#define PLICSW_CONTEXT_CLAIM 0x4

#define PLICSW_HART_MASK 0x01010101

#define PLICSW_HART_MAX_NR 8

#define PLICSW_REGION_ALIGN 0x1000

struct plicsw_data {
	unsigned long addr;
	unsigned long size;
	uint32_t hart_count;
	/* hart id to source id table */
	uint32_t source_id[PLICSW_HART_MAX_NR];
};

int plicsw_warm_ipi_init(void);

int plicsw_cold_ipi_init(struct plicsw_data *plicsw);

#endif /* _IPI_ANDES_PLICSW_H_ */

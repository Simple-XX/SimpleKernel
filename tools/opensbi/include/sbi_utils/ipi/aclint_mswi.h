/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __IPI_ACLINT_MSWI_H__
#define __IPI_ACLINT_MSWI_H__

#include <sbi/sbi_types.h>

#define ACLINT_MSWI_ALIGN		0x1000
#define ACLINT_MSWI_SIZE		0x4000
#define ACLINT_MSWI_MAX_HARTS		4095

#define CLINT_MSWI_OFFSET		0x0000

struct aclint_mswi_data {
	/* Public details */
	unsigned long addr;
	unsigned long size;
	u32 first_hartid;
	u32 hart_count;
};

int aclint_mswi_warm_init(void);

int aclint_mswi_cold_init(struct aclint_mswi_data *mswi);

#endif

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __FDT_RESET_H__
#define __FDT_RESET_H__

#include <sbi/sbi_types.h>

struct fdt_reset {
	const struct fdt_match *match_table;
	int (*init)(void *fdt, int nodeoff, const struct fdt_match *match);
};

#ifdef CONFIG_FDT_RESET

/**
 * fdt_reset_driver_init() - initialize reset driver based on the device-tree
 */
int fdt_reset_driver_init(void *fdt, struct fdt_reset *drv);

/**
 * fdt_reset_init() - initialize reset drivers based on the device-tree
 *
 * This function shall be invoked in final init.
 */
void fdt_reset_init(void);

#else

static inline int fdt_reset_driver_init(void *fdt, struct fdt_reset *drv)
{
	return 0;
}
static inline void fdt_reset_init(void) { }

#endif

#endif

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 YADRO
 *
 * Authors:
 *   Nikita Shubin <n.shubin@yadro.com>
 */

#ifndef __FDT_I2C_H__
#define __FDT_I2C_H__

#include <sbi_utils/i2c/i2c.h>

/** FDT based I2C adapter driver */
struct fdt_i2c_adapter {
	const struct fdt_match *match_table;
	int (*init)(void *fdt, int nodeoff,
		    const struct fdt_match *match);
};

/** Get I2C adapter identified by nodeoff */
int fdt_i2c_adapter_get(void *fdt, int nodeoff,
			struct i2c_adapter **out_adapter);

#endif

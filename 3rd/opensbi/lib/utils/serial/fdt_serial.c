/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <libfdt.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_scratch.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>

/* List of FDT serial drivers generated at compile time */
extern struct fdt_serial *fdt_serial_drivers[];
extern unsigned long fdt_serial_drivers_size;

static struct fdt_serial dummy = {
	.match_table = NULL,
	.init = NULL,
};

static struct fdt_serial *current_driver = &dummy;

int fdt_serial_init(void)
{
	const void *prop;
	struct fdt_serial *drv;
	const struct fdt_match *match;
	int pos, noff = -1, len, coff, rc;
	void *fdt = fdt_get_address();

	/* Find offset of node pointed to by stdout-path */
	coff = fdt_path_offset(fdt, "/chosen");
	if (-1 < coff) {
		prop = fdt_getprop(fdt, coff, "stdout-path", &len);
		if (prop && len) {
			const char *sep, *start = prop;

			/* The device path may be followed by ':' */
			sep = strchr(start, ':');
			if (sep)
				noff = fdt_path_offset_namelen(fdt, prop,
							       sep - start);
			else
				noff = fdt_path_offset(fdt, prop);
		}
	}

	/* First check DT node pointed by stdout-path */
	for (pos = 0; pos < fdt_serial_drivers_size && -1 < noff; pos++) {
		drv = fdt_serial_drivers[pos];

		match = fdt_match_node(fdt, noff, drv->match_table);
		if (!match)
			continue;

		if (drv->init) {
			rc = drv->init(fdt, noff, match);
			if (rc == SBI_ENODEV)
				continue;
			if (rc)
				return rc;
		}
		current_driver = drv;
		break;
	}

	/* Check if we found desired driver */
	if (current_driver != &dummy)
		goto done;

	/* Lastly check all DT nodes */
	for (pos = 0; pos < fdt_serial_drivers_size; pos++) {
		drv = fdt_serial_drivers[pos];

		noff = fdt_find_match(fdt, -1, drv->match_table, &match);
		if (noff < 0)
			continue;

		if (drv->init) {
			rc = drv->init(fdt, noff, match);
			if (rc == SBI_ENODEV)
				continue;
			if (rc)
				return rc;
		}
		current_driver = drv;
		break;
	}

done:
	return 0;
}

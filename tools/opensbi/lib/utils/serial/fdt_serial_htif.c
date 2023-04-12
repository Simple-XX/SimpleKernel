/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/sys/htif.h>

static const struct fdt_match serial_htif_match[] = {
	{ .compatible = "ucb,htif0" },
	{ },
};

static int serial_htif_init(void *fdt, int nodeoff,
			    const struct fdt_match *match)
{
	bool custom = false;
	uint64_t fromhost_addr = 0, tohost_addr = 0;

	if (!fdt_get_node_addr_size(fdt, nodeoff, 0, &fromhost_addr, NULL)) {
		custom = true;
		tohost_addr = fromhost_addr + sizeof(uint64_t);
	}

	fdt_get_node_addr_size(fdt, nodeoff, 1, &tohost_addr, NULL);

	return htif_serial_init(custom, fromhost_addr, tohost_addr);
}

struct fdt_serial fdt_serial_htif = {
	.match_table = serial_htif_match,
	.init = serial_htif_init
};

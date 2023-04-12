/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Alistair Francis <alistair.francis@wdc.com>
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/serial/xlnx_uartlite.h>

static int serial_xlnx_uartlite_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	int rc;
	struct platform_uart_data uart = { 0 };

	rc = fdt_parse_xlnx_uartlite_node(fdt, nodeoff, &uart);
	if (rc)
		return rc;

	return xlnx_uartlite_init(uart.addr);
}

static const struct fdt_match serial_xlnx_uartlite_match[] = {
	{ .compatible = "xlnx,xps-uartlite-1.00.a" },
	{ },
};

struct fdt_serial fdt_serial_xlnx_uartlite = {
	.match_table = serial_xlnx_uartlite_match,
	.init = serial_xlnx_uartlite_init,
};

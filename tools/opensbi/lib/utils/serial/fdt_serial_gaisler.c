/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Cobham Gaisler AB.
 *
 * Authors:
 *   Daniel Cederman <cederman@gaisler.com>
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/serial/gaisler-uart.h>

static int serial_gaisler_init(void *fdt, int nodeoff,
			       const struct fdt_match *match)
{
	int rc;
	struct platform_uart_data uart = { 0 };

	rc = fdt_parse_gaisler_uart_node(fdt, nodeoff, &uart);
	if (rc)
		return rc;

	return gaisler_uart_init(uart.addr, uart.freq, uart.baud);
}

static const struct fdt_match serial_gaisler_match[] = {
	{ .compatible = "gaisler,apbuart" },
	{},
};

struct fdt_serial fdt_serial_gaisler = {
	.match_table = serial_gaisler_match,
	.init = serial_gaisler_init
};

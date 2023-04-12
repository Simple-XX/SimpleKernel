// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2022 Renesas Electronics Corporation
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/serial/renesas-scif.h>

static int serial_renesas_scif_init(void *fdt, int nodeoff,
				    const struct fdt_match *match)
{
	struct platform_uart_data uart = { 0 };
	int ret;

	ret = fdt_parse_renesas_scif_node(fdt, nodeoff, &uart);
	if (ret)
		return ret;

	return renesas_scif_init(uart.addr, uart.freq, uart.baud);
}

static const struct fdt_match serial_renesas_scif_match[] = {
	{ .compatible = "renesas,scif-r9a07g043" },
	{ /* sentinel */ }
};

struct fdt_serial fdt_serial_renesas_scif = {
	.match_table = serial_renesas_scif_match,
	.init = serial_renesas_scif_init
};

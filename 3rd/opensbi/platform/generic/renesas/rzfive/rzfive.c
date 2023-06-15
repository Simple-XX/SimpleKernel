// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Renesas Electronics Corp.
 *
 */

#include <platform_override.h>
#include <sbi_utils/fdt/fdt_helper.h>

static const struct fdt_match renesas_rzfive_match[] = {
	{ .compatible = "renesas,r9a07g043f01" },
	{ /* sentinel */ }
};

const struct platform_override renesas_rzfive = {
	.match_table = renesas_rzfive_match,
};

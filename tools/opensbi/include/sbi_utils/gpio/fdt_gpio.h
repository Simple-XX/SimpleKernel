/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __FDT_GPIO_H__
#define __FDT_GPIO_H__

#include <sbi_utils/gpio/gpio.h>

struct fdt_phandle_args;

/** FDT based GPIO driver */
struct fdt_gpio {
	const struct fdt_match *match_table;
	int (*xlate)(struct gpio_chip *chip,
		     const struct fdt_phandle_args *pargs,
		     struct gpio_pin *out_pin);
	int (*init)(void *fdt, int nodeoff, u32 phandle,
		    const struct fdt_match *match);
};

/** Get a GPIO pin using "gpios" DT property of client DT node */
int fdt_gpio_pin_get(void *fdt, int nodeoff, int index,
		     struct gpio_pin *out_pin);

/** Simple xlate function to convert two GPIO FDT cells into GPIO pin */
int fdt_gpio_simple_xlate(struct gpio_chip *chip,
			  const struct fdt_phandle_args *pargs,
			  struct gpio_pin *out_pin);

#endif

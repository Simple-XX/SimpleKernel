/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_error.h>
#include <sbi_utils/gpio/gpio.h>

static SBI_LIST_HEAD(gpio_chip_list);

struct gpio_chip *gpio_chip_find(unsigned int id)
{
	struct sbi_dlist *pos;

	sbi_list_for_each(pos, &(gpio_chip_list)) {
		struct gpio_chip *chip = to_gpio_chip(pos);

		if (chip->id == id)
			return chip;
	}

	return NULL;
}

int gpio_chip_add(struct gpio_chip *gc)
{
	if (!gc)
		return SBI_EINVAL;
	if (gpio_chip_find(gc->id))
		return SBI_EALREADY;

	sbi_list_add(&(gc->node), &(gpio_chip_list));

	return 0;
}

void gpio_chip_remove(struct gpio_chip *gc)
{
	if (!gc)
		return;

	sbi_list_del(&(gc->node));
}

int gpio_get_direction(struct gpio_pin *gp)
{
	if (!gp || !gp->chip || (gp->chip->ngpio <= gp->offset))
		return SBI_EINVAL;
	if (!gp->chip->get_direction)
		return SBI_ENOSYS;

	return gp->chip->get_direction(gp);
}

int gpio_direction_input(struct gpio_pin *gp)
{
	if (!gp || !gp->chip || (gp->chip->ngpio <= gp->offset))
		return SBI_EINVAL;
	if (!gp->chip->direction_input)
		return SBI_ENOSYS;

	return gp->chip->direction_input(gp);
}

int gpio_direction_output(struct gpio_pin *gp, int value)
{
	if (!gp || !gp->chip || (gp->chip->ngpio <= gp->offset))
		return SBI_EINVAL;
	if (!gp->chip->direction_output)
		return SBI_ENOSYS;

	return gp->chip->direction_output(gp, value);
}

int gpio_get(struct gpio_pin *gp)
{
	if (!gp || !gp->chip || (gp->chip->ngpio <= gp->offset))
		return SBI_EINVAL;
	if (!gp->chip->get)
		return SBI_ENOSYS;

	return gp->chip->get(gp);
}

int gpio_set(struct gpio_pin *gp, int value)
{
	if (!gp || !gp->chip || (gp->chip->ngpio <= gp->offset))
		return SBI_EINVAL;
	if (!gp->chip->set)
		return SBI_ENOSYS;

	gp->chip->set(gp, value);
	return 0;
}

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#include <sbi/sbi_types.h>
#include <sbi/sbi_list.h>

#define GPIO_LINE_DIRECTION_IN	1
#define GPIO_LINE_DIRECTION_OUT	0

/** Representation of a GPIO pin */
struct gpio_pin {
	/** Pointer to the GPIO chip */
	struct gpio_chip *chip;
	/** Identification of GPIO pin within GPIO chip */
	unsigned int offset;
	/**
	 * Additional configuration flags of the GPIO pin desired
	 * by GPIO clients.
	 *
	 * NOTE: GPIO chip can have custom configuration flags.
	 */
	unsigned int flags;
#define GPIO_FLAG_ACTIVE_LOW	0x1
#define GPIO_FLAG_SINGLE_ENDED	0x2
#define GPIO_FLAG_OPEN_DRAIN	0x4
#define GPIO_FLAG_TRANSITORY	0x8
#define GPIO_FLAG_PULL_UP	0x10
#define GPIO_FLAG_PULL_DOWN	0x20
};

/** Representation of a GPIO chip */
struct gpio_chip {
	/** Pointer to GPIO driver owning this GPIO chip */
	void *driver;
	/** Uniquie ID of the GPIO chip assigned by the driver */
	unsigned int id;
	/** Number of GPIOs supported by the GPIO chip */
	unsigned int ngpio;
	/**
	 * Get current direction of GPIO pin
	 *
	 * @return 0=output, 1=input, or negative error
	 */
	int (*get_direction)(struct gpio_pin *gp);
	/**
	 * Set input direction of GPIO pin
	 *
	 * @return 0 on success and negative error code on failure
	 */
	int (*direction_input)(struct gpio_pin *gp);
	/**
	 * Set output direction of GPIO pin with given output value
	 *
	 * @return 0 on success and negative error code on failure
	 */
	int (*direction_output)(struct gpio_pin *gp, int value);
	/**
	 * Get current value of GPIO pin
	 *
	 * @return 0=low, 1=high, or negative error
	 */
	int (*get)(struct gpio_pin *gp);
	/** Set output value for GPIO pin */
	void (*set)(struct gpio_pin *gp, int value);
	/** List */
	struct sbi_dlist node;
};

static inline struct gpio_chip *to_gpio_chip(struct sbi_dlist *node)
{
	return container_of(node, struct gpio_chip, node);
}

/** Find a registered GPIO chip */
struct gpio_chip *gpio_chip_find(unsigned int id);

/** Register GPIO chip */
int gpio_chip_add(struct gpio_chip *gc);

/** Un-register GPIO chip */
void gpio_chip_remove(struct gpio_chip *gc);

/** Get current direction of GPIO pin */
int gpio_get_direction(struct gpio_pin *gp);

/** Set input direction of GPIO pin */
int gpio_direction_input(struct gpio_pin *gp);

/** Set output direction of GPIO pin */
int gpio_direction_output(struct gpio_pin *gp, int value);

/** Get current value of GPIO pin */
int gpio_get(struct gpio_pin *gp);

/** Set output value of GPIO pin */
int gpio_set(struct gpio_pin *gp, int value);

#endif

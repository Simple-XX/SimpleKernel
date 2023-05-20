#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2021 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbiutils-objs-$(CONFIG_FDT_GPIO) += gpio/fdt_gpio.o
libsbiutils-objs-$(CONFIG_FDT_GPIO) += gpio/fdt_gpio_drivers.o

carray-fdt_gpio_drivers-$(CONFIG_FDT_GPIO_SIFIVE) += fdt_gpio_sifive
libsbiutils-objs-$(CONFIG_FDT_GPIO_SIFIVE) += gpio/fdt_gpio_sifive.o

libsbiutils-objs-$(CONFIG_GPIO) += gpio/gpio.o

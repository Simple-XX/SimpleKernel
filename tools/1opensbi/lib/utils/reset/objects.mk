#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2020 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbiutils-objs-$(CONFIG_FDT_RESET) += reset/fdt_reset.o
libsbiutils-objs-$(CONFIG_FDT_RESET) += reset/fdt_reset_drivers.o

carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_ATCWDT200) += fdt_reset_atcwdt200
libsbiutils-objs-$(CONFIG_FDT_RESET_ATCWDT200) += reset/fdt_reset_atcwdt200.o

carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_GPIO) += fdt_poweroff_gpio
carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_GPIO) += fdt_reset_gpio
libsbiutils-objs-$(CONFIG_FDT_RESET_GPIO) += reset/fdt_reset_gpio.o

carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_HTIF) += fdt_reset_htif
libsbiutils-objs-$(CONFIG_FDT_RESET_HTIF) += reset/fdt_reset_htif.o

carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_SIFIVE_TEST) += fdt_reset_sifive_test
libsbiutils-objs-$(CONFIG_FDT_RESET_SIFIVE_TEST) += reset/fdt_reset_sifive_test.o

carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_SUNXI_WDT) += fdt_reset_sunxi_wdt
libsbiutils-objs-$(CONFIG_FDT_RESET_SUNXI_WDT) += reset/fdt_reset_sunxi_wdt.o

carray-fdt_reset_drivers-$(CONFIG_FDT_RESET_THEAD) += fdt_reset_thead
libsbiutils-objs-$(CONFIG_FDT_RESET_THEAD) += reset/fdt_reset_thead.o
libsbiutils-objs-$(CONFIG_FDT_RESET_THEAD) += reset/fdt_reset_thead_asm.o

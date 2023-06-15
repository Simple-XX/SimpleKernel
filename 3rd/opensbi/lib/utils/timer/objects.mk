#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2020 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbiutils-objs-$(CONFIG_TIMER_MTIMER) += timer/aclint_mtimer.o
libsbiutils-objs-$(CONFIG_TIMER_PLMT) += timer/andes_plmt.o

libsbiutils-objs-$(CONFIG_FDT_TIMER) += timer/fdt_timer.o
libsbiutils-objs-$(CONFIG_FDT_TIMER) += timer/fdt_timer_drivers.o

carray-fdt_timer_drivers-$(CONFIG_FDT_TIMER_MTIMER) += fdt_timer_mtimer
libsbiutils-objs-$(CONFIG_FDT_TIMER_MTIMER) += timer/fdt_timer_mtimer.o

carray-fdt_timer_drivers-$(CONFIG_FDT_TIMER_PLMT) += fdt_timer_plmt
libsbiutils-objs-$(CONFIG_FDT_TIMER_PLMT) += timer/fdt_timer_plmt.o

#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (C) 2020 Bin Meng <bmeng.cn@gmail.com>
#

libsbiutils-objs-$(CONFIG_FDT_DOMAIN) += fdt/fdt_domain.o
libsbiutils-objs-$(CONFIG_FDT_PMU) += fdt/fdt_pmu.o
libsbiutils-objs-$(CONFIG_FDT) += fdt/fdt_helper.o
libsbiutils-objs-$(CONFIG_FDT) += fdt/fdt_fixup.o

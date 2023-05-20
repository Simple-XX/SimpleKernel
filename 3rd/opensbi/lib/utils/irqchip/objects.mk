#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbiutils-objs-$(CONFIG_FDT_IRQCHIP) += irqchip/fdt_irqchip.o
libsbiutils-objs-$(CONFIG_FDT_IRQCHIP) += irqchip/fdt_irqchip_drivers.o

carray-fdt_irqchip_drivers-$(CONFIG_FDT_IRQCHIP_APLIC) += fdt_irqchip_aplic
libsbiutils-objs-$(CONFIG_FDT_IRQCHIP_APLIC) += irqchip/fdt_irqchip_aplic.o

carray-fdt_irqchip_drivers-$(CONFIG_FDT_IRQCHIP_IMSIC) += fdt_irqchip_imsic
libsbiutils-objs-$(CONFIG_FDT_IRQCHIP_IMSIC) += irqchip/fdt_irqchip_imsic.o

carray-fdt_irqchip_drivers-$(CONFIG_FDT_IRQCHIP_PLIC) += fdt_irqchip_plic
libsbiutils-objs-$(CONFIG_FDT_IRQCHIP_PLIC) += irqchip/fdt_irqchip_plic.o

libsbiutils-objs-$(CONFIG_IRQCHIP_APLIC) += irqchip/aplic.o
libsbiutils-objs-$(CONFIG_IRQCHIP_IMSIC) += irqchip/imsic.o
libsbiutils-objs-$(CONFIG_IRQCHIP_PLIC) += irqchip/plic.o

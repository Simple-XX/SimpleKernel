#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2021 YADRO
#
# Authors:
#   Nikita Shubin <n.shubin@yadro.com>
#

libsbiutils-objs-$(CONFIG_I2C) += i2c/i2c.o

libsbiutils-objs-$(CONFIG_FDT_I2C) += i2c/fdt_i2c.o
libsbiutils-objs-$(CONFIG_FDT_I2C) += i2c/fdt_i2c_adapter_drivers.o

carray-fdt_i2c_adapter_drivers-$(CONFIG_FDT_I2C_SIFIVE) += fdt_i2c_adapter_sifive
libsbiutils-objs-$(CONFIG_FDT_I2C_SIFIVE) += i2c/fdt_i2c_sifive.o

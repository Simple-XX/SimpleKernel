#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbiutils-objs-$(CONFIG_SYS_HTIF) += sys/htif.o
libsbiutils-objs-$(CONFIG_SYS_SIFIVE_TEST) += sys/sifive_test.o

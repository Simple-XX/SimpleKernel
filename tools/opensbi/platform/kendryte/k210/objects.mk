#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Damien Le Moal <damien.lemoal@wdc.com>
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Objects to build
platform-objs-y += platform.o

platform-objs-y += k210.o
platform-varprefix-k210.o = dt_k210
platform-padding-k210.o = 2048

# Blobs to build
FW_TEXT_START=0x80000000
FW_PAYLOAD=y
FW_PAYLOAD_ALIGN=0x1000

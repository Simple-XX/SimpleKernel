#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2020 Nuclei Corporation or its affiliates.
#
# Authors:
#   lujun <lujun@nuclesys.com>
#   hqfang <578567190@qq.com>
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Command for platform specific "make run"
platform-runcmd = xl_spike \
  $(build_dir)/platform/nuclei/ux600/firmware/fw_payload.elf

# Objects to build
platform-objs-y += platform.o

# Blobs to build
FW_TEXT_START=0xA0000000
FW_DYNAMIC=y
FW_JUMP=y

FW_JUMP_ADDR=0xA0200000
FW_JUMP_FDT_ADDR=0xA8000000
FW_PAYLOAD=y
FW_PAYLOAD_OFFSET=0x200000
FW_PAYLOAD_FDT_ADDR=0xA8000000

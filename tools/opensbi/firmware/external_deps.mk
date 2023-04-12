#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

$(platform_build_dir)/firmware/fw_dynamic.o: $(FW_FDT_PATH)
$(platform_build_dir)/firmware/fw_jump.o: $(FW_FDT_PATH)
$(platform_build_dir)/firmware/fw_payload.o: $(FW_FDT_PATH)

$(platform_build_dir)/firmware/fw_payload.o: $(FW_PAYLOAD_PATH_FINAL)

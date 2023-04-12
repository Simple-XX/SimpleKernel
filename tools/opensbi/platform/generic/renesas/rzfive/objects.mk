#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (C) 2022 Renesas Electronics Corp.
#

carray-platform_override_modules-$(CONFIG_PLATFORM_RENESAS_RZFIVE) += renesas_rzfive
platform-objs-$(CONFIG_PLATFORM_RENESAS_RZFIVE) += renesas/rzfive/rzfive.o

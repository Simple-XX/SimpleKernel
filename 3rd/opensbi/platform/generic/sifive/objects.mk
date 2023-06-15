#
# SPDX-License-Identifier: BSD-2-Clause
#

carray-platform_override_modules-$(CONFIG_PLATFORM_SIFIVE_FU540) += sifive_fu540
platform-objs-$(CONFIG_PLATFORM_SIFIVE_FU540) += sifive/fu540.o

carray-platform_override_modules-$(CONFIG_PLATFORM_SIFIVE_FU740) += sifive_fu740
platform-objs-$(CONFIG_PLATFORM_SIFIVE_FU740) += sifive/fu740.o

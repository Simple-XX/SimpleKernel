#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2021 Jessica Clarke <jrtc27@jrtc27.com>
#

ifeq ($(PLATFORM_RISCV_XLEN),32)
libsbiutils-objs-y += libquad/divdi3.o
libsbiutils-objs-y += libquad/moddi3.o
libsbiutils-objs-y += libquad/qdivrem.o
libsbiutils-objs-y += libquad/udivdi3.o
libsbiutils-objs-y += libquad/umoddi3.o
libsbiutils-genflags-y += -I$(libsbiutils_dir)/libquad/include
endif

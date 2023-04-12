#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbi-objs-y += riscv_asm.o
libsbi-objs-y += riscv_atomic.o
libsbi-objs-y += riscv_hardfp.o
libsbi-objs-y += riscv_locks.o

libsbi-objs-y += sbi_ecall.o
libsbi-objs-y += sbi_ecall_exts.o

# The order of below extensions is performance optimized
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_TIME) += ecall_time
libsbi-objs-$(CONFIG_SBI_ECALL_TIME) += sbi_ecall_time.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_RFENCE) += ecall_rfence
libsbi-objs-$(CONFIG_SBI_ECALL_RFENCE) += sbi_ecall_rfence.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_IPI) += ecall_ipi
libsbi-objs-$(CONFIG_SBI_ECALL_IPI) += sbi_ecall_ipi.o

carray-sbi_ecall_exts-y += ecall_base
libsbi-objs-y += sbi_ecall_base.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_HSM) += ecall_hsm
libsbi-objs-$(CONFIG_SBI_ECALL_HSM) += sbi_ecall_hsm.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_SRST) += ecall_srst
libsbi-objs-$(CONFIG_SBI_ECALL_SRST) += sbi_ecall_srst.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_PMU) += ecall_pmu
libsbi-objs-$(CONFIG_SBI_ECALL_PMU) += sbi_ecall_pmu.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_LEGACY) += ecall_legacy
libsbi-objs-$(CONFIG_SBI_ECALL_LEGACY) += sbi_ecall_legacy.o

carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_VENDOR) += ecall_vendor
libsbi-objs-$(CONFIG_SBI_ECALL_VENDOR) += sbi_ecall_vendor.o

libsbi-objs-y += sbi_bitmap.o
libsbi-objs-y += sbi_bitops.o
libsbi-objs-y += sbi_console.o
libsbi-objs-y += sbi_domain.o
libsbi-objs-y += sbi_emulate_csr.o
libsbi-objs-y += sbi_fifo.o
libsbi-objs-y += sbi_hart.o
libsbi-objs-y += sbi_math.o
libsbi-objs-y += sbi_hfence.o
libsbi-objs-y += sbi_hsm.o
libsbi-objs-y += sbi_illegal_insn.o
libsbi-objs-y += sbi_init.o
libsbi-objs-y += sbi_ipi.o
libsbi-objs-y += sbi_irqchip.o
libsbi-objs-y += sbi_misaligned_ldst.o
libsbi-objs-y += sbi_platform.o
libsbi-objs-y += sbi_pmu.o
libsbi-objs-y += sbi_scratch.o
libsbi-objs-y += sbi_string.o
libsbi-objs-y += sbi_system.o
libsbi-objs-y += sbi_timer.o
libsbi-objs-y += sbi_tlb.o
libsbi-objs-y += sbi_trap.o
libsbi-objs-y += sbi_unpriv.o
libsbi-objs-y += sbi_expected_trap.o

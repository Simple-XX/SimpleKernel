/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __IRQCHIP_IMSIC_H__
#define __IRQCHIP_IMSIC_H__

#include <sbi/sbi_types.h>

#define IMSIC_MMIO_PAGE_SHIFT		12
#define IMSIC_MMIO_PAGE_SZ		(1UL << IMSIC_MMIO_PAGE_SHIFT)

#define IMSIC_MAX_REGS			16

struct imsic_regs {
	unsigned long addr;
	unsigned long size;
};

struct imsic_data {
	bool targets_mmode;
	u32 guest_index_bits;
	u32 hart_index_bits;
	u32 group_index_bits;
	u32 group_index_shift;
	unsigned long num_ids;
	struct imsic_regs regs[IMSIC_MAX_REGS];
};

#ifdef CONFIG_IRQCHIP_IMSIC

int imsic_map_hartid_to_data(u32 hartid, struct imsic_data *imsic, int file);

struct imsic_data *imsic_get_data(u32 hartid);

int imsic_get_target_file(u32 hartid);

void imsic_local_irqchip_init(void);

int imsic_warm_irqchip_init(void);

int imsic_data_check(struct imsic_data *imsic);

int imsic_cold_irqchip_init(struct imsic_data *imsic);

#else

static inline void imsic_local_irqchip_init(void) { }

static inline int imsic_data_check(struct imsic_data *imsic) { return 0; }

#endif

#endif

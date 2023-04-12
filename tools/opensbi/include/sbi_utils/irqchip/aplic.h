/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __IRQCHIP_APLIC_H__
#define __IRQCHIP_APLIC_H__

#include <sbi/sbi_types.h>

#define APLIC_MAX_DELEGATE	16

struct aplic_msicfg_data {
	unsigned long lhxs;
	unsigned long lhxw;
	unsigned long hhxs;
	unsigned long hhxw;
	unsigned long base_addr;
};

struct aplic_delegate_data {
	u32 first_irq;
	u32 last_irq;
	u32 child_index;
};

struct aplic_data {
	unsigned long addr;
	unsigned long size;
	unsigned long num_idc;
	unsigned long num_source;
	bool targets_mmode;
	bool has_msicfg_mmode;
	struct aplic_msicfg_data msicfg_mmode;
	bool has_msicfg_smode;
	struct aplic_msicfg_data msicfg_smode;
	struct aplic_delegate_data delegate[APLIC_MAX_DELEGATE];
};

int aplic_cold_irqchip_init(struct aplic_data *aplic);

#endif

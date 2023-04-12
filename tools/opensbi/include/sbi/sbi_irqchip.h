/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <apatel@ventanamicro.com>
 */

#ifndef __SBI_IRQCHIP_H__
#define __SBI_IRQCHIP_H__

#include <sbi/sbi_types.h>

struct sbi_scratch;
struct sbi_trap_regs;

/**
 * Set external interrupt handling function
 *
 * This function is called by OpenSBI platform code to set a handler for
 * external interrupts
 *
 * @param fn function pointer for handling external irqs
 */
void sbi_irqchip_set_irqfn(int (*fn)(struct sbi_trap_regs *regs));

/**
 * Process external interrupts
 *
 * This function is called by sbi_trap_handler() to handle external
 * interrupts.
 *
 * @param regs pointer for trap registers
 */
int sbi_irqchip_process(struct sbi_trap_regs *regs);

/** Initialize interrupt controllers */
int sbi_irqchip_init(struct sbi_scratch *scratch, bool cold_boot);

/** Exit interrupt controllers */
void sbi_irqchip_exit(struct sbi_scratch *scratch);

#endif

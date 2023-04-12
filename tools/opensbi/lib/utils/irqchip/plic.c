/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *   Samuel Holland <samuel@sholland.org>
 */

#include <sbi/riscv_io.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_string.h>
#include <sbi_utils/irqchip/plic.h>

#define PLIC_PRIORITY_BASE 0x0
#define PLIC_PENDING_BASE 0x1000
#define PLIC_ENABLE_BASE 0x2000
#define PLIC_ENABLE_STRIDE 0x80
#define PLIC_CONTEXT_BASE 0x200000
#define PLIC_CONTEXT_STRIDE 0x1000

static u32 plic_get_priority(const struct plic_data *plic, u32 source)
{
	volatile void *plic_priority = (char *)plic->addr +
			PLIC_PRIORITY_BASE + 4 * source;
	return readl(plic_priority);
}

static void plic_set_priority(const struct plic_data *plic, u32 source, u32 val)
{
	volatile void *plic_priority = (char *)plic->addr +
			PLIC_PRIORITY_BASE + 4 * source;
	writel(val, plic_priority);
}

void plic_priority_save(const struct plic_data *plic, u8 *priority, u32 num)
{
	for (u32 i = 1; i <= num; i++)
		priority[i] = plic_get_priority(plic, i);
}

void plic_priority_restore(const struct plic_data *plic, const u8 *priority,
			   u32 num)
{
	for (u32 i = 1; i <= num; i++)
		plic_set_priority(plic, i, priority[i]);
}

static u32 plic_get_thresh(const struct plic_data *plic, u32 cntxid)
{
	volatile void *plic_thresh;

	plic_thresh = (char *)plic->addr +
		      PLIC_CONTEXT_BASE + PLIC_CONTEXT_STRIDE * cntxid;

	return readl(plic_thresh);
}

static void plic_set_thresh(const struct plic_data *plic, u32 cntxid, u32 val)
{
	volatile void *plic_thresh;

	plic_thresh = (char *)plic->addr +
		      PLIC_CONTEXT_BASE + PLIC_CONTEXT_STRIDE * cntxid;
	writel(val, plic_thresh);
}

static u32 plic_get_ie(const struct plic_data *plic, u32 cntxid,
		       u32 word_index)
{
	volatile void *plic_ie;

	plic_ie = (char *)plic->addr +
		   PLIC_ENABLE_BASE + PLIC_ENABLE_STRIDE * cntxid +
		   4 * word_index;

	return readl(plic_ie);
}

static void plic_set_ie(const struct plic_data *plic, u32 cntxid,
			u32 word_index, u32 val)
{
	volatile void *plic_ie;

	plic_ie = (char *)plic->addr +
		   PLIC_ENABLE_BASE + PLIC_ENABLE_STRIDE * cntxid +
		   4 * word_index;
	writel(val, plic_ie);
}

void plic_context_save(const struct plic_data *plic, int context_id,
		       u32 *enable, u32 *threshold, u32 num)
{
	u32 ie_words = plic->num_src / 32 + 1;

	if (num > ie_words)
		num = ie_words;

	for (u32 i = 0; i < num; i++)
		enable[i] = plic_get_ie(plic, context_id, i);

	*threshold = plic_get_thresh(plic, context_id);
}

void plic_context_restore(const struct plic_data *plic, int context_id,
			  const u32 *enable, u32 threshold, u32 num)
{
	u32 ie_words = plic->num_src / 32 + 1;

	if (num > ie_words)
		num = ie_words;

	for (u32 i = 0; i < num; i++)
		plic_set_ie(plic, context_id, i, enable[i]);

	plic_set_thresh(plic, context_id, threshold);
}

int plic_context_init(const struct plic_data *plic, int context_id,
		      bool enable, u32 threshold)
{
	u32 ie_words, ie_value;

	if (!plic || context_id < 0)
		return SBI_EINVAL;

	ie_words = plic->num_src / 32 + 1;
	ie_value = enable ? 0xffffffffU : 0U;

	for (u32 i = 0; i < ie_words; i++)
		plic_set_ie(plic, context_id, i, ie_value);

	plic_set_thresh(plic, context_id, threshold);

	return 0;
}

int plic_warm_irqchip_init(const struct plic_data *plic,
			   int m_cntx_id, int s_cntx_id)
{
	int ret;

	/* By default, disable all IRQs for M-mode of target HART */
	if (m_cntx_id > -1) {
		ret = plic_context_init(plic, m_cntx_id, false, 0x7);
		if (ret)
			return ret;
	}

	/* By default, disable all IRQs for S-mode of target HART */
	if (s_cntx_id > -1) {
		ret = plic_context_init(plic, s_cntx_id, false, 0x7);
		if (ret)
			return ret;
	}

	return 0;
}

int plic_cold_irqchip_init(const struct plic_data *plic)
{
	int i;

	if (!plic)
		return SBI_EINVAL;

	/* Configure default priorities of all IRQs */
	for (i = 1; i <= plic->num_src; i++)
		plic_set_priority(plic, i, 0);

	return 0;
}

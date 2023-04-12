/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Andes Technology Corporation
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 *   Leo Yu-Chi Liang <ycliang@andestech.com>
 *   Yu Chien Peter Lin <peterlin@andestech.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_ipi.h>
#include <sbi_utils/ipi/andes_plicsw.h>

struct plicsw_data plicsw;

static inline void plicsw_claim(void)
{
	u32 hartid = current_hartid();

	if (plicsw.hart_count <= hartid)
		ebreak();

	plicsw.source_id[hartid] =
		readl((void *)plicsw.addr + PLICSW_CONTEXT_BASE +
		      PLICSW_CONTEXT_CLAIM + PLICSW_CONTEXT_STRIDE * hartid);
}

static inline void plicsw_complete(void)
{
	u32 hartid = current_hartid();
	u32 source = plicsw.source_id[hartid];

	writel(source, (void *)plicsw.addr + PLICSW_CONTEXT_BASE +
			       PLICSW_CONTEXT_CLAIM +
			       PLICSW_CONTEXT_STRIDE * hartid);
}

static inline void plic_sw_pending(u32 target_hart)
{
	/*
	 * The pending array registers are w1s type.
	 * IPI pending array mapping as following:
	 *
	 * Pending array start address: base + 0x1000
	 * ---------------------------------
	 * | hart3 | hart2 | hart1 | hart0 |
	 * ---------------------------------
	 * Each hartX can send IPI to another hart by setting the
	 * bitY to its own region (see the below).
	 *
	 * In each hartX region:
	 * <---------- PICSW_PENDING_STRIDE -------->
	 * | bit7 | ... | bit3 | bit2 | bit1 | bit0 |
	 * ------------------------------------------
	 * The bitY of hartX region indicates that hartX sends an
	 * IPI to hartY.
	 */
	u32 hartid	    = current_hartid();
	u32 word_index	    = hartid / 4;
	u32 per_hart_offset = PLICSW_PENDING_STRIDE * hartid;
	u32 val		    = 1 << target_hart << per_hart_offset;

	writel(val, (void *)plicsw.addr + PLICSW_PENDING_BASE + word_index * 4);
}

static void plicsw_ipi_send(u32 target_hart)
{
	if (plicsw.hart_count <= target_hart)
		ebreak();

	/* Set PLICSW IPI */
	plic_sw_pending(target_hart);
}

static void plicsw_ipi_clear(u32 target_hart)
{
	if (plicsw.hart_count <= target_hart)
		ebreak();

	/* Clear PLICSW IPI */
	plicsw_claim();
	plicsw_complete();
}

static struct sbi_ipi_device plicsw_ipi = {
	.name      = "andes_plicsw",
	.ipi_send  = plicsw_ipi_send,
	.ipi_clear = plicsw_ipi_clear
};

int plicsw_warm_ipi_init(void)
{
	u32 hartid = current_hartid();

	/* Clear PLICSW IPI */
	plicsw_ipi_clear(hartid);

	return 0;
}

int plicsw_cold_ipi_init(struct plicsw_data *plicsw)
{
	int rc;

	/* Setup source priority */
	uint32_t *priority = (void *)plicsw->addr + PLICSW_PRIORITY_BASE;

	for (int i = 0; i < plicsw->hart_count; i++)
		writel(1, &priority[i]);

	/* Setup target enable */
	uint32_t enable_mask = PLICSW_HART_MASK;

	for (int i = 0; i < plicsw->hart_count; i++) {
		uint32_t *enable = (void *)plicsw->addr + PLICSW_ENABLE_BASE +
				   PLICSW_ENABLE_STRIDE * i;
		writel(enable_mask, enable);
		writel(enable_mask, enable + 1);
		enable_mask <<= 1;
	}

	/* Add PLICSW region to the root domain */
	rc = sbi_domain_root_add_memrange(plicsw->addr, plicsw->size,
					  PLICSW_REGION_ALIGN,
					  SBI_DOMAIN_MEMREGION_MMIO);
	if (rc)
		return rc;

	sbi_ipi_set_device(&plicsw_ipi);

	return 0;
}

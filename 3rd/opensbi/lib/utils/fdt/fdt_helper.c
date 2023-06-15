// SPDX-License-Identifier: BSD-2-Clause
/*
 * fdt_helper.c - Flat Device Tree manipulation helper routines
 * Implement helper routines on top of libfdt for OpenSBI usage
 *
 * Copyright (C) 2020 Bin Meng <bmeng.cn@gmail.com>
 */

#include <libfdt.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_scratch.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/irqchip/aplic.h>
#include <sbi_utils/irqchip/imsic.h>
#include <sbi_utils/irqchip/plic.h>

#define DEFAULT_UART_FREQ		0
#define DEFAULT_UART_BAUD		115200
#define DEFAULT_UART_REG_SHIFT		0
#define DEFAULT_UART_REG_IO_WIDTH	1
#define DEFAULT_UART_REG_OFFSET		0

#define DEFAULT_RENESAS_SCIF_FREQ		100000000
#define DEFAULT_RENESAS_SCIF_BAUD		115200

#define DEFAULT_SIFIVE_UART_FREQ		0
#define DEFAULT_SIFIVE_UART_BAUD		115200

#define DEFAULT_SHAKTI_UART_FREQ		50000000
#define DEFAULT_SHAKTI_UART_BAUD		115200

const struct fdt_match *fdt_match_node(void *fdt, int nodeoff,
				       const struct fdt_match *match_table)
{
	int ret;

	if (!fdt || nodeoff < 0 || !match_table)
		return NULL;

	while (match_table->compatible) {
		ret = fdt_node_check_compatible(fdt, nodeoff,
						match_table->compatible);
		if (!ret)
			return match_table;
		match_table++;
	}

	return NULL;
}

int fdt_find_match(void *fdt, int startoff,
		   const struct fdt_match *match_table,
		   const struct fdt_match **out_match)
{
	int nodeoff;

	if (!fdt || !match_table)
		return SBI_ENODEV;

	while (match_table->compatible) {
		nodeoff = fdt_node_offset_by_compatible(fdt, startoff,
						match_table->compatible);
		if (nodeoff >= 0) {
			if (out_match)
				*out_match = match_table;
			return nodeoff;
		}
		match_table++;
	}

	return SBI_ENODEV;
}

int fdt_parse_phandle_with_args(void *fdt, int nodeoff,
				const char *prop, const char *cells_prop,
				int index, struct fdt_phandle_args *out_args)
{
	u32 i, pcells;
	int len, pnodeoff;
	const fdt32_t *list, *list_end, *val;

	if (!fdt || (nodeoff < 0) || !prop || !cells_prop || !out_args)
		return SBI_EINVAL;

	list = fdt_getprop(fdt, nodeoff, prop, &len);
	if (!list)
		return SBI_ENOENT;
	list_end = list + (len / sizeof(*list));

	while (list < list_end) {
		pnodeoff = fdt_node_offset_by_phandle(fdt,
						fdt32_to_cpu(*list));
		if (pnodeoff < 0)
			return pnodeoff;
		list++;

		val = fdt_getprop(fdt, pnodeoff, cells_prop, &len);
		if (!val)
			return SBI_ENOENT;
		pcells = fdt32_to_cpu(*val);
		if (FDT_MAX_PHANDLE_ARGS < pcells)
			return SBI_EINVAL;
		if (list + pcells > list_end)
			return SBI_ENOENT;

		if (index > 0) {
			list += pcells;
			index--;
		} else {
			out_args->node_offset = pnodeoff;
			out_args->args_count = pcells;
			for (i = 0; i < pcells; i++)
				out_args->args[i] = fdt32_to_cpu(list[i]);
			return 0;
		}
	}

	return SBI_ENOENT;
}

static int fdt_translate_address(void *fdt, uint64_t reg, int parent,
				 uint64_t *addr)
{
	int i, rlen;
	int cell_addr, cell_size;
	const fdt32_t *ranges;
	uint64_t offset, caddr = 0, paddr = 0, rsize = 0;

	cell_addr = fdt_address_cells(fdt, parent);
	if (cell_addr < 1)
		return SBI_ENODEV;

	cell_size = fdt_size_cells(fdt, parent);
	if (cell_size < 0)
		return SBI_ENODEV;

	ranges = fdt_getprop(fdt, parent, "ranges", &rlen);
	if (ranges && rlen > 0) {
		for (i = 0; i < cell_addr; i++)
			caddr = (caddr << 32) | fdt32_to_cpu(*ranges++);
		for (i = 0; i < cell_addr; i++)
			paddr = (paddr << 32) | fdt32_to_cpu(*ranges++);
		for (i = 0; i < cell_size; i++)
			rsize = (rsize << 32) | fdt32_to_cpu(*ranges++);
		if (reg < caddr || caddr >= (reg + rsize )) {
			sbi_printf("invalid address translation\n");
			return SBI_ENODEV;
		}
		offset = reg - caddr;
		*addr = paddr + offset;
	} else {
		/* No translation required */
		*addr = reg;
	}

	return 0;
}

int fdt_get_node_addr_size(void *fdt, int node, int index,
			   uint64_t *addr, uint64_t *size)
{
	int parent, len, i, rc;
	int cell_addr, cell_size;
	const fdt32_t *prop_addr, *prop_size;
	uint64_t temp = 0;

	if (!fdt || node < 0 || index < 0)
		return SBI_EINVAL;

	parent = fdt_parent_offset(fdt, node);
	if (parent < 0)
		return parent;
	cell_addr = fdt_address_cells(fdt, parent);
	if (cell_addr < 1)
		return SBI_ENODEV;

	cell_size = fdt_size_cells(fdt, parent);
	if (cell_size < 0)
		return SBI_ENODEV;

	prop_addr = fdt_getprop(fdt, node, "reg", &len);
	if (!prop_addr)
		return SBI_ENODEV;

	if ((len / sizeof(u32)) <= (index * (cell_addr + cell_size)))
		return SBI_EINVAL;

	prop_addr = prop_addr + (index * (cell_addr + cell_size));
	prop_size = prop_addr + cell_addr;

	if (addr) {
		for (i = 0; i < cell_addr; i++)
			temp = (temp << 32) | fdt32_to_cpu(*prop_addr++);
		do {
			if (parent < 0)
				break;
			rc  = fdt_translate_address(fdt, temp, parent, addr);
			if (rc)
				break;
			parent = fdt_parent_offset(fdt, parent);
			temp = *addr;
		} while (1);
	}
	temp = 0;

	if (size) {
		for (i = 0; i < cell_size; i++)
			temp = (temp << 32) | fdt32_to_cpu(*prop_size++);
		*size = temp;
	}

	return 0;
}

bool fdt_node_is_enabled(void *fdt, int nodeoff)
{
	int len;
	const void *prop;

	prop = fdt_getprop(fdt, nodeoff, "status", &len);
	if (!prop)
		return true;

	if (!strncmp(prop, "okay", strlen("okay")))
		return true;

	if (!strncmp(prop, "ok", strlen("ok")))
		return true;

	return false;
}

int fdt_parse_hart_id(void *fdt, int cpu_offset, u32 *hartid)
{
	int len;
	const void *prop;
	const fdt32_t *val;

	if (!fdt || cpu_offset < 0)
		return SBI_EINVAL;

	prop = fdt_getprop(fdt, cpu_offset, "device_type", &len);
	if (!prop || !len)
		return SBI_EINVAL;
	if (strncmp (prop, "cpu", strlen ("cpu")))
		return SBI_EINVAL;

	val = fdt_getprop(fdt, cpu_offset, "reg", &len);
	if (!val || len < sizeof(fdt32_t))
		return SBI_EINVAL;

	if (len > sizeof(fdt32_t))
		val++;

	if (hartid)
		*hartid = fdt32_to_cpu(*val);

	return 0;
}

int fdt_parse_max_enabled_hart_id(void *fdt, u32 *max_hartid)
{
	u32 hartid;
	int err, cpu_offset, cpus_offset;

	if (!fdt)
		return SBI_EINVAL;
	if (!max_hartid)
		return 0;

	*max_hartid = 0;

	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		return cpus_offset;

	fdt_for_each_subnode(cpu_offset, fdt, cpus_offset) {
		err = fdt_parse_hart_id(fdt, cpu_offset, &hartid);
		if (err)
			continue;

		if (!fdt_node_is_enabled(fdt, cpu_offset))
			continue;

		if (hartid > *max_hartid)
			*max_hartid = hartid;
	}

	return 0;
}

int fdt_parse_timebase_frequency(void *fdt, unsigned long *freq)
{
	const fdt32_t *val;
	int len, cpus_offset;

	if (!fdt || !freq)
		return SBI_EINVAL;

	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		return cpus_offset;

	val = fdt_getprop(fdt, cpus_offset, "timebase-frequency", &len);
	if (len > 0 && val)
		*freq = fdt32_to_cpu(*val);
	else
		return SBI_ENOENT;

	return 0;
}

static int fdt_parse_uart_node_common(void *fdt, int nodeoffset,
				      struct platform_uart_data *uart,
				      unsigned long default_freq,
				      unsigned long default_baud)
{
	int len, rc;
	const fdt32_t *val;
	uint64_t reg_addr, reg_size;

	if (nodeoffset < 0 || !uart || !fdt)
		return SBI_ENODEV;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0 || !reg_addr || !reg_size)
		return SBI_ENODEV;
	uart->addr = reg_addr;

	/**
	 * UART address is mandatory. clock-frequency and current-speed
	 * may not be present. Don't return error.
	 */
	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "clock-frequency", &len);
	if (len > 0 && val)
		uart->freq = fdt32_to_cpu(*val);
	else
		uart->freq = default_freq;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "current-speed", &len);
	if (len > 0 && val)
		uart->baud = fdt32_to_cpu(*val);
	else
		uart->baud = default_baud;

	return 0;
}

int fdt_parse_gaisler_uart_node(void *fdt, int nodeoffset,
				struct platform_uart_data *uart)
{
	return fdt_parse_uart_node_common(fdt, nodeoffset, uart,
					DEFAULT_UART_FREQ,
					DEFAULT_UART_BAUD);
}

int fdt_parse_renesas_scif_node(void *fdt, int nodeoffset,
				struct platform_uart_data *uart)
{
	return fdt_parse_uart_node_common(fdt, nodeoffset, uart,
					  DEFAULT_RENESAS_SCIF_FREQ,
					  DEFAULT_RENESAS_SCIF_BAUD);
}

int fdt_parse_shakti_uart_node(void *fdt, int nodeoffset,
			       struct platform_uart_data *uart)
{
	return fdt_parse_uart_node_common(fdt, nodeoffset, uart,
					DEFAULT_SHAKTI_UART_FREQ,
					DEFAULT_SHAKTI_UART_BAUD);
}

int fdt_parse_sifive_uart_node(void *fdt, int nodeoffset,
			       struct platform_uart_data *uart)
{
	return fdt_parse_uart_node_common(fdt, nodeoffset, uart,
					DEFAULT_SIFIVE_UART_FREQ,
					DEFAULT_SIFIVE_UART_BAUD);
}

int fdt_parse_uart_node(void *fdt, int nodeoffset,
			struct platform_uart_data *uart)
{
	int len, rc;
	const fdt32_t *val;

	rc = fdt_parse_uart_node_common(fdt, nodeoffset, uart,
					DEFAULT_UART_FREQ,
					DEFAULT_UART_BAUD);
	if (rc)
		return rc;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "reg-shift", &len);
	if (len > 0 && val)
		uart->reg_shift = fdt32_to_cpu(*val);
	else
		uart->reg_shift = DEFAULT_UART_REG_SHIFT;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "reg-io-width", &len);
	if (len > 0 && val)
		uart->reg_io_width = fdt32_to_cpu(*val);
	else
		uart->reg_io_width = DEFAULT_UART_REG_IO_WIDTH;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "reg-offset", &len);
	if (len > 0 && val)
		uart->reg_offset = fdt32_to_cpu(*val);
	else
		uart->reg_offset = DEFAULT_UART_REG_OFFSET;

	return 0;
}

int fdt_parse_uart8250(void *fdt, struct platform_uart_data *uart,
		   const char *compatible)
{
	int nodeoffset;

	if (!compatible || !uart || !fdt)
		return SBI_ENODEV;

	nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compatible);
	if (nodeoffset < 0)
		return nodeoffset;

	return fdt_parse_uart_node(fdt, nodeoffset, uart);
}

int fdt_parse_xlnx_uartlite_node(void *fdt, int nodeoffset,
			       struct platform_uart_data *uart)
{
	return fdt_parse_uart_node_common(fdt, nodeoffset, uart, 0, 0);
}

int fdt_parse_aplic_node(void *fdt, int nodeoff, struct aplic_data *aplic)
{
	bool child_found;
	const fdt32_t *val;
	const fdt32_t *del;
	struct imsic_data imsic;
	int i, j, d, dcnt, len, noff, rc;
	uint64_t reg_addr, reg_size;
	struct aplic_delegate_data *deleg;

	if (nodeoff < 0 || !aplic || !fdt)
		return SBI_ENODEV;
	memset(aplic, 0, sizeof(*aplic));

	rc = fdt_get_node_addr_size(fdt, nodeoff, 0, &reg_addr, &reg_size);
	if (rc < 0 || !reg_addr || !reg_size)
		return SBI_ENODEV;
	aplic->addr = reg_addr;
	aplic->size = reg_size;

	val = fdt_getprop(fdt, nodeoff, "riscv,num-sources", &len);
	if (len > 0)
		aplic->num_source = fdt32_to_cpu(*val);

	val = fdt_getprop(fdt, nodeoff, "interrupts-extended", &len);
	if (val && len > sizeof(fdt32_t)) {
		len = len / sizeof(fdt32_t);
		for (i = 0; i < len; i += 2) {
			if (fdt32_to_cpu(val[i + 1]) == IRQ_M_EXT) {
				aplic->targets_mmode = true;
				break;
			}
		}
		aplic->num_idc = len / 2;
		goto aplic_msi_parent_done;
	}

	val = fdt_getprop(fdt, nodeoff, "msi-parent", &len);
	if (val && len >= sizeof(fdt32_t)) {
		noff = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*val));
		if (noff < 0)
			return noff;

		rc = fdt_parse_imsic_node(fdt, noff, &imsic);
		if (rc)
			return rc;

		rc = imsic_data_check(&imsic);
		if (rc)
			return rc;

		aplic->targets_mmode = imsic.targets_mmode;

		if (imsic.targets_mmode) {
			aplic->has_msicfg_mmode = true;
			aplic->msicfg_mmode.lhxs = imsic.guest_index_bits;
			aplic->msicfg_mmode.lhxw = imsic.hart_index_bits;
			aplic->msicfg_mmode.hhxw = imsic.group_index_bits;
			aplic->msicfg_mmode.hhxs = imsic.group_index_shift;
			if (aplic->msicfg_mmode.hhxs <
					(2 * IMSIC_MMIO_PAGE_SHIFT))
				return SBI_EINVAL;
			aplic->msicfg_mmode.hhxs -= 24;
			aplic->msicfg_mmode.base_addr = imsic.regs[0].addr;
		} else {
			goto aplic_msi_parent_done;
		}

		val = fdt_getprop(fdt, nodeoff, "riscv,children", &len);
		if (!val || len < sizeof(fdt32_t))
			goto aplic_msi_parent_done;

		noff = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*val));
		if (noff < 0)
			return noff;

		val = fdt_getprop(fdt, noff, "msi-parent", &len);
		if (!val || len < sizeof(fdt32_t))
			goto aplic_msi_parent_done;

		noff = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*val));
		if (noff < 0)
			return noff;

		rc = fdt_parse_imsic_node(fdt, noff, &imsic);
		if (rc)
			return rc;

		rc = imsic_data_check(&imsic);
		if (rc)
			return rc;

		if (!imsic.targets_mmode) {
			aplic->has_msicfg_smode = true;
			aplic->msicfg_smode.lhxs = imsic.guest_index_bits;
			aplic->msicfg_smode.lhxw = imsic.hart_index_bits;
			aplic->msicfg_smode.hhxw = imsic.group_index_bits;
			aplic->msicfg_smode.hhxs = imsic.group_index_shift;
			if (aplic->msicfg_smode.hhxs <
					(2 * IMSIC_MMIO_PAGE_SHIFT))
				return SBI_EINVAL;
			aplic->msicfg_smode.hhxs -= 24;
			aplic->msicfg_smode.base_addr = imsic.regs[0].addr;
		}
	}
aplic_msi_parent_done:

	for (d = 0; d < APLIC_MAX_DELEGATE; d++) {
		deleg = &aplic->delegate[d];
		deleg->first_irq = 0;
		deleg->last_irq = 0;
		deleg->child_index = 0;
	}

	del = fdt_getprop(fdt, nodeoff, "riscv,delegate", &len);
	if (!del || len < (3 * sizeof(fdt32_t)))
		goto skip_delegate_parse;
	d = 0;
	dcnt = len / sizeof(fdt32_t);
	for (i = 0; i < dcnt; i += 3) {
		if (d >= APLIC_MAX_DELEGATE)
			break;
		deleg = &aplic->delegate[d];

		deleg->first_irq = fdt32_to_cpu(del[i + 1]);
		deleg->last_irq = fdt32_to_cpu(del[i + 2]);
		deleg->child_index = 0;

		child_found = false;
		val = fdt_getprop(fdt, nodeoff, "riscv,children", &len);
		if (!val || len < sizeof(fdt32_t)) {
			deleg->first_irq = 0;
			deleg->last_irq = 0;
			deleg->child_index = 0;
			continue;
		}
		len = len / sizeof(fdt32_t);
		for (j = 0; j < len; j++) {
			if (del[i] != val[j])
				continue;
			deleg->child_index = j;
			child_found = true;
			break;
		}

		if (child_found) {
			d++;
		} else {
			deleg->first_irq = 0;
			deleg->last_irq = 0;
			deleg->child_index = 0;
		}
	}
skip_delegate_parse:

	return 0;
}

bool fdt_check_imsic_mlevel(void *fdt)
{
	const fdt32_t *val;
	int i, len, noff = 0;

	if (!fdt)
		return false;

	while ((noff = fdt_node_offset_by_compatible(fdt, noff,
						     "riscv,imsics")) >= 0) {
		val = fdt_getprop(fdt, noff, "interrupts-extended", &len);
		if (val && len > sizeof(fdt32_t)) {
			len = len / sizeof(fdt32_t);
			for (i = 0; i < len; i += 2) {
				if (fdt32_to_cpu(val[i + 1]) == IRQ_M_EXT)
					return true;
			}
		}
	}

	return false;
}

int fdt_parse_imsic_node(void *fdt, int nodeoff, struct imsic_data *imsic)
{
	const fdt32_t *val;
	struct imsic_regs *regs;
	uint64_t reg_addr, reg_size;
	int i, rc, len, nr_parent_irqs;

	if (nodeoff < 0 || !imsic || !fdt)
		return SBI_ENODEV;

	imsic->targets_mmode = false;
	val = fdt_getprop(fdt, nodeoff, "interrupts-extended", &len);
	if (val && len > sizeof(fdt32_t)) {
		len = len / sizeof(fdt32_t);
		nr_parent_irqs = len / 2;
		for (i = 0; i < len; i += 2) {
			if (fdt32_to_cpu(val[i + 1]) == IRQ_M_EXT) {
				imsic->targets_mmode = true;
				break;
			}
		}
	} else
		return SBI_EINVAL;

	val = fdt_getprop(fdt, nodeoff, "riscv,guest-index-bits", &len);
	if (val && len > 0)
		imsic->guest_index_bits = fdt32_to_cpu(*val);
	else
		imsic->guest_index_bits = 0;

	val = fdt_getprop(fdt, nodeoff, "riscv,hart-index-bits", &len);
	if (val && len > 0) {
		imsic->hart_index_bits = fdt32_to_cpu(*val);
	} else {
		imsic->hart_index_bits = sbi_fls(nr_parent_irqs);
		if ((1UL << imsic->hart_index_bits) < nr_parent_irqs)
			imsic->hart_index_bits++;
	}

	val = fdt_getprop(fdt, nodeoff, "riscv,group-index-bits", &len);
	if (val && len > 0)
		imsic->group_index_bits = fdt32_to_cpu(*val);
	else
		imsic->group_index_bits = 0;

	val = fdt_getprop(fdt, nodeoff, "riscv,group-index-shift", &len);
	if (val && len > 0)
		imsic->group_index_shift = fdt32_to_cpu(*val);
	else
		imsic->group_index_shift = 2 * IMSIC_MMIO_PAGE_SHIFT;

	val = fdt_getprop(fdt, nodeoff, "riscv,num-ids", &len);
	if (val && len > 0)
		imsic->num_ids = fdt32_to_cpu(*val);
	else
		return SBI_EINVAL;

	for (i = 0; i < IMSIC_MAX_REGS; i++) {
		regs = &imsic->regs[i];
		regs->addr = 0;
		regs->size = 0;
	}

	for (i = 0; i < (IMSIC_MAX_REGS - 1); i++) {
		regs = &imsic->regs[i];

		rc = fdt_get_node_addr_size(fdt, nodeoff, i,
					    &reg_addr, &reg_size);
		if (rc < 0 || !reg_addr || !reg_size)
			break;
		regs->addr = reg_addr;
		regs->size = reg_size;
	};
	if (!imsic->regs[0].size)
		return SBI_EINVAL;

	return 0;
}

int fdt_parse_plic_node(void *fdt, int nodeoffset, struct plic_data *plic)
{
	int len, rc;
	const fdt32_t *val;
	uint64_t reg_addr, reg_size;

	if (nodeoffset < 0 || !plic || !fdt)
		return SBI_ENODEV;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0 || !reg_addr || !reg_size)
		return SBI_ENODEV;
	plic->addr = reg_addr;

	val = fdt_getprop(fdt, nodeoffset, "riscv,ndev", &len);
	if (len > 0)
		plic->num_src = fdt32_to_cpu(*val);

	return 0;
}

int fdt_parse_plic(void *fdt, struct plic_data *plic, const char *compat)
{
	int nodeoffset;

	if (!compat || !plic || !fdt)
		return SBI_ENODEV;

	nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compat);
	if (nodeoffset < 0)
		return nodeoffset;

	return fdt_parse_plic_node(fdt, nodeoffset, plic);
}

int fdt_parse_aclint_node(void *fdt, int nodeoffset, bool for_timer,
			  unsigned long *out_addr1, unsigned long *out_size1,
			  unsigned long *out_addr2, unsigned long *out_size2,
			  u32 *out_first_hartid, u32 *out_hart_count)
{
	const fdt32_t *val;
	uint64_t reg_addr, reg_size;
	int i, rc, count, cpu_offset, cpu_intc_offset;
	u32 phandle, hwirq, hartid, first_hartid, last_hartid, hart_count;
	u32 match_hwirq = (for_timer) ? IRQ_M_TIMER : IRQ_M_SOFT;

	if (nodeoffset < 0 || !fdt ||
	    !out_addr1 || !out_size1 ||
	    !out_first_hartid || !out_hart_count)
		return SBI_EINVAL;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0 || !reg_size)
		return SBI_ENODEV;
	*out_addr1 = reg_addr;
	*out_size1 = reg_size;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 1,
				    &reg_addr, &reg_size);
	if (rc < 0 || !reg_size)
		reg_addr = reg_size = 0;
	if (out_addr2)
		*out_addr2 = reg_addr;
	if (out_size2)
		*out_size2 = reg_size;

	*out_first_hartid = 0;
	*out_hart_count = 0;

	val = fdt_getprop(fdt, nodeoffset, "interrupts-extended", &count);
	if (!val || count < sizeof(fdt32_t))
		return 0;
	count = count / sizeof(fdt32_t);

	first_hartid = -1U;
	hart_count = last_hartid = 0;
	for (i = 0; i < (count / 2); i++) {
		phandle = fdt32_to_cpu(val[2 * i]);
		hwirq = fdt32_to_cpu(val[(2 * i) + 1]);

		cpu_intc_offset = fdt_node_offset_by_phandle(fdt, phandle);
		if (cpu_intc_offset < 0)
			continue;

		cpu_offset = fdt_parent_offset(fdt, cpu_intc_offset);
		if (cpu_offset < 0)
			continue;

		rc = fdt_parse_hart_id(fdt, cpu_offset, &hartid);
		if (rc)
			continue;

		if (SBI_HARTMASK_MAX_BITS <= hartid)
			continue;

		if (match_hwirq == hwirq) {
			if (hartid < first_hartid)
				first_hartid = hartid;
			if (hartid > last_hartid)
				last_hartid = hartid;
			hart_count++;
		}
	}

	if ((last_hartid >= first_hartid) && first_hartid != -1U) {
		*out_first_hartid = first_hartid;
		count = last_hartid - first_hartid + 1;
		*out_hart_count = (hart_count < count) ? hart_count : count;
	}

	return 0;
}

int fdt_parse_plmt_node(void *fdt, int nodeoffset, unsigned long *plmt_base,
			  unsigned long *plmt_size, u32 *hart_count)
{
	const fdt32_t *val;
	int rc, i, count;
	uint64_t reg_addr, reg_size;
	u32 phandle, hwirq, hartid, hcount;

	if (nodeoffset < 0 || !fdt || !plmt_base ||
	    !hart_count || !plmt_size)
		return SBI_EINVAL;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0)
		return SBI_ENODEV;
	*plmt_base = reg_addr;
	*plmt_size = reg_size;

	val = fdt_getprop(fdt, nodeoffset, "interrupts-extended", &count);
	if (!val || count < sizeof(fdt32_t))
		return 0;
	count = count / sizeof(fdt32_t);

	hcount = 0;
	for (i = 0; i < (count / 2); i++) {
		int cpu_offset, cpu_intc_offset;

		phandle = fdt32_to_cpu(val[2 * i]);
		hwirq = fdt32_to_cpu(val[2 * i + 1]);

		cpu_intc_offset = fdt_node_offset_by_phandle(fdt, phandle);
		if (cpu_intc_offset < 0)
			continue;

		cpu_offset = fdt_parent_offset(fdt, cpu_intc_offset);
		if (cpu_offset < 0)
			continue;

		rc = fdt_parse_hart_id(fdt, cpu_offset, &hartid);

		if (rc)
			continue;

		if (SBI_HARTMASK_MAX_BITS <= hartid)
			continue;

		if (hwirq == IRQ_M_TIMER)
			hcount++;
	}

	*hart_count = hcount;

	return 0;
}

int fdt_parse_plicsw_node(void *fdt, int nodeoffset, unsigned long *plicsw_base,
			  unsigned long *size, u32 *hart_count)
{
	const fdt32_t *val;
	int rc, i, count;
	uint64_t reg_addr, reg_size;
	u32 phandle, hwirq, hartid, hcount;

	if (nodeoffset < 0 || !fdt || !plicsw_base ||
	    !hart_count || !size)
		return SBI_EINVAL;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0)
		return SBI_ENODEV;
	*plicsw_base = reg_addr;
	*size = reg_size;

	val = fdt_getprop(fdt, nodeoffset, "interrupts-extended", &count);
	if (!val || count < sizeof(fdt32_t))
		return 0;
	count = count / sizeof(fdt32_t);

	hcount = 0;
	for (i = 0; i < (count / 2); i++) {
		int cpu_offset, cpu_intc_offset;

		phandle = fdt32_to_cpu(val[2 * i]);
		hwirq = fdt32_to_cpu(val[2 * i + 1]);

		cpu_intc_offset = fdt_node_offset_by_phandle(fdt, phandle);
		if (cpu_intc_offset < 0)
			continue;

		cpu_offset = fdt_parent_offset(fdt, cpu_intc_offset);
		if (cpu_offset < 0)
			continue;

		rc = fdt_parse_hart_id(fdt, cpu_offset, &hartid);

		if (rc)
			continue;

		if (SBI_HARTMASK_MAX_BITS <= hartid)
			continue;

		if (hwirq == IRQ_M_SOFT)
			hcount++;
	}

	*hart_count = hcount;

	return 0;
}

int fdt_parse_compat_addr(void *fdt, uint64_t *addr,
			  const char *compatible)
{
	int nodeoffset, rc;

	nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compatible);
	if (nodeoffset < 0)
		return nodeoffset;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0, addr, NULL);
	if (rc < 0 || !addr)
		return SBI_ENODEV;

	return 0;
}

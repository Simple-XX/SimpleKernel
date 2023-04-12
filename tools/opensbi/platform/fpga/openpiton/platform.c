// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_platform.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/ipi/aclint_mswi.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/uart8250.h>
#include <sbi_utils/timer/aclint_mtimer.h>

#define OPENPITON_DEFAULT_UART_ADDR		0xfff0c2c000
#define OPENPITON_DEFAULT_UART_FREQ		60000000
#define OPENPITON_DEFAULT_UART_BAUDRATE		115200
#define OPENPITON_DEFAULT_UART_REG_SHIFT	0
#define OPENPITON_DEFAULT_UART_REG_WIDTH	1
#define OPENPITON_DEFAULT_UART_REG_OFFSET	0
#define OPENPITON_DEFAULT_PLIC_ADDR		0xfff1100000
#define OPENPITON_DEFAULT_PLIC_NUM_SOURCES	2
#define OPENPITON_DEFAULT_HART_COUNT		3
#define OPENPITON_DEFAULT_CLINT_ADDR		0xfff1020000
#define OPENPITON_DEFAULT_ACLINT_MTIMER_FREQ	1000000
#define OPENPITON_DEFAULT_ACLINT_MSWI_ADDR	\
		(OPENPITON_DEFAULT_CLINT_ADDR + CLINT_MSWI_OFFSET)
#define OPENPITON_DEFAULT_ACLINT_MTIMER_ADDR	\
		(OPENPITON_DEFAULT_CLINT_ADDR + CLINT_MTIMER_OFFSET)

static struct platform_uart_data uart = {
	OPENPITON_DEFAULT_UART_ADDR,
	OPENPITON_DEFAULT_UART_FREQ,
	OPENPITON_DEFAULT_UART_BAUDRATE,
};
static struct plic_data plic = {
	.addr = OPENPITON_DEFAULT_PLIC_ADDR,
	.num_src = OPENPITON_DEFAULT_PLIC_NUM_SOURCES,
};

static struct aclint_mswi_data mswi = {
	.addr = OPENPITON_DEFAULT_ACLINT_MSWI_ADDR,
	.size = ACLINT_MSWI_SIZE,
	.first_hartid = 0,
	.hart_count = OPENPITON_DEFAULT_HART_COUNT,
};

static struct aclint_mtimer_data mtimer = {
	.mtime_freq = OPENPITON_DEFAULT_ACLINT_MTIMER_FREQ,
	.mtime_addr = OPENPITON_DEFAULT_ACLINT_MTIMER_ADDR +
		      ACLINT_DEFAULT_MTIME_OFFSET,
	.mtime_size = ACLINT_DEFAULT_MTIME_SIZE,
	.mtimecmp_addr = OPENPITON_DEFAULT_ACLINT_MTIMER_ADDR +
			 ACLINT_DEFAULT_MTIMECMP_OFFSET,
	.mtimecmp_size = ACLINT_DEFAULT_MTIMECMP_SIZE,
	.first_hartid = 0,
	.hart_count = OPENPITON_DEFAULT_HART_COUNT,
	.has_64bit_mmio = TRUE,
};

/*
 * OpenPiton platform early initialization.
 */
static int openpiton_early_init(bool cold_boot)
{
	void *fdt;
	struct platform_uart_data uart_data = { 0 };
	struct plic_data plic_data;
	unsigned long aclint_freq;
	uint64_t clint_addr;
	int rc;

	if (!cold_boot)
		return 0;
	fdt = fdt_get_address();

	rc = fdt_parse_uart8250(fdt, &uart_data, "ns16550");
	if (!rc)
		uart = uart_data;

	rc = fdt_parse_plic(fdt, &plic_data, "riscv,plic0");
	if (!rc)
		plic = plic_data;

	rc = fdt_parse_timebase_frequency(fdt, &aclint_freq);
	if (!rc)
		mtimer.mtime_freq = aclint_freq;

	rc = fdt_parse_compat_addr(fdt, &clint_addr, "riscv,clint0");
	if (!rc) {
		mswi.addr = clint_addr;
		mtimer.mtime_addr = clint_addr + CLINT_MTIMER_OFFSET +
				    ACLINT_DEFAULT_MTIME_OFFSET;
		mtimer.mtimecmp_addr = clint_addr + CLINT_MTIMER_OFFSET +
				    ACLINT_DEFAULT_MTIMECMP_OFFSET;
	}

	return 0;
}

/*
 * OpenPiton platform final initialization.
 */
static int openpiton_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = fdt_get_address();
	fdt_fixups(fdt);

	return 0;
}

/*
 * Initialize the openpiton console.
 */
static int openpiton_console_init(void)
{
	return uart8250_init(uart.addr,
			     uart.freq,
			     uart.baud,
			     OPENPITON_DEFAULT_UART_REG_SHIFT,
			     OPENPITON_DEFAULT_UART_REG_WIDTH,
			     OPENPITON_DEFAULT_UART_REG_OFFSET);
}

static int plic_openpiton_warm_irqchip_init(int m_cntx_id, int s_cntx_id)
{
	int ret;

	/* By default, enable all IRQs for M-mode of target HART */
	if (m_cntx_id > -1) {
		ret = plic_context_init(&plic, m_cntx_id, true, 0x1);
		if (ret)
			return ret;
	}

	/* Enable all IRQs for S-mode of target HART */
	if (s_cntx_id > -1) {
		ret = plic_context_init(&plic, s_cntx_id, true, 0x0);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Initialize the openpiton interrupt controller for current HART.
 */
static int openpiton_irqchip_init(bool cold_boot)
{
	u32 hartid = current_hartid();
	int ret;

	if (cold_boot) {
		ret = plic_cold_irqchip_init(&plic);
		if (ret)
			return ret;
	}
	return plic_openpiton_warm_irqchip_init(2 * hartid, 2 * hartid + 1);
}

/*
 * Initialize IPI for current HART.
 */
static int openpiton_ipi_init(bool cold_boot)
{
	int ret;

	if (cold_boot) {
		ret = aclint_mswi_cold_init(&mswi);
		if (ret)
			return ret;
	}

	return aclint_mswi_warm_init();
}

/*
 * Initialize openpiton timer for current HART.
 */
static int openpiton_timer_init(bool cold_boot)
{
	int ret;

	if (cold_boot) {
		ret = aclint_mtimer_cold_init(&mtimer, NULL);
		if (ret)
			return ret;
	}

	return aclint_mtimer_warm_init();
}

/*
 * Platform descriptor.
 */
const struct sbi_platform_operations platform_ops = {
	.early_init = openpiton_early_init,
	.final_init = openpiton_final_init,
	.console_init = openpiton_console_init,
	.irqchip_init = openpiton_irqchip_init,
	.ipi_init = openpiton_ipi_init,
	.timer_init = openpiton_timer_init,
};

const struct sbi_platform platform = {
	.opensbi_version = OPENSBI_VERSION,
	.platform_version = SBI_PLATFORM_VERSION(0x0, 0x01),
	.name = "OPENPITON RISC-V",
	.features = SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count = OPENPITON_DEFAULT_HART_COUNT,
	.hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.platform_ops_addr = (unsigned long)&platform_ops
};

// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2022 Renesas Electronics Corporation
 */

#include <sbi/riscv_io.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_timer.h>
#include <sbi_utils/serial/renesas-scif.h>

/* clang-format off */

#define SCIF_REG_SMR		0x0
#define SCIF_REG_BRR		0x2
#define SCIF_REG_SCR		0x4
#define SCIF_REG_FTDR		0x6
#define SCIF_REG_FSR		0x8
#define SCIF_REG_FCR		0xc
#define SCIF_REG_LSR		0x12
#define SCIF_REG_SEMR		0x14

#define SCIF_FCR_RFRST		0x2 /* Reset assert receive-FIFO (bit[1]) */
#define SCIF_FCR_TFRST		0x4 /* Reset assert transmit-FIFO(bit[2]) */

#define SCIF_FCR_RST_ASSRT_RFTF	(SCIF_FCR_RFRST | SCIF_FCR_TFRST) /* Reset assert tx-FIFO & rx-FIFO */
#define SCIF_FCR_RST_NGATE_RFTF	0x0 /* Reset negate tx-FIFO & rx-FIFO */

#define SCIF_SCR_RE		0x10 /* Enable receive (bit[4]) */
#define SCIF_SCR_TE		0x20 /* Enable transmit(bit[5]) */
#define SCIF_SCR_RCV_TRN_EN	(SCIF_SCR_RE | SCIF_SCR_TE) /* Enable receive & transmit */
#define SCIF_SCR_RCV_TRN_DIS	0x0 /* Disable receive & transmit */

#define SCIF_FSR_ER		0x80 /* Receive error flag */
#define SCIF_FSR_TEND		0x40 /* Transmit End Flag */
#define SCIF_FSR_TDFE		0x20 /* Transmit FIFO Data Empty Flag */
#define SCIF_FSR_BRK		0x10 /* Detect break flag */
#define SCIF_FSR_DR		0x1  /* Receive data ready flag */

#define SCIF_FSR_TXD_CHK	(SCIF_FSR_TEND | SCIF_FSR_TDFE)

#define SCIF_SEMR_MDDRS		0x10 /* MDDR access enable */

#define SCIF_REG_8BIT(reg)	((reg == SCIF_REG_BRR) || \
				 (reg == SCIF_REG_FTDR) || \
				 (reg == SCIF_REG_SEMR))

#define SCBRR_VALUE(clk, baudrate) ((clk) / (32 * (baudrate)) - 1)

/* clang-format on */

static volatile char *scif_base;

static u32 get_reg(u32 offset)
{
	if (SCIF_REG_8BIT(offset))
		return readb(scif_base + offset);

	return readw(scif_base + offset);
}

static void set_reg(u32 offset, u32 val)
{
	if (SCIF_REG_8BIT(offset))
		writeb(val, scif_base + offset);
	else
		writew(val, scif_base + offset);
}

static void renesas_scif_putc(char ch)
{
	uint16_t reg;

	while (!(SCIF_FSR_TXD_CHK & get_reg(SCIF_REG_FSR)))
		;

	set_reg(SCIF_REG_FTDR, ch);
	reg = get_reg(SCIF_REG_FSR);
	reg &= ~SCIF_FSR_TXD_CHK;
	set_reg(SCIF_REG_FSR, reg);
}

static struct sbi_console_device renesas_scif_console = {
	.name		= "renesas_scif",
	.console_putc	= renesas_scif_putc,
};

int renesas_scif_init(unsigned long base, u32 in_freq, u32 baudrate)
{
	uint16_t data16;

	scif_base = (volatile char *)base;

	set_reg(SCIF_REG_SCR, SCIF_SCR_RCV_TRN_DIS);
	set_reg(SCIF_REG_FCR, SCIF_FCR_RST_ASSRT_RFTF);

	data16 = get_reg(SCIF_REG_FSR); /* Dummy read */
	set_reg(SCIF_REG_FSR, 0x0); /* Clear all error bit */

	data16 = get_reg(SCIF_REG_LSR); /* Dummy read */
	set_reg(SCIF_REG_LSR, 0x0); /* Clear ORER bit */

	set_reg(SCIF_REG_SCR, 0x0);

	set_reg(SCIF_REG_SMR, 0x0);

	data16 = get_reg(SCIF_REG_SEMR);
	set_reg(SCIF_REG_SEMR, data16 & (~SCIF_SEMR_MDDRS));
	set_reg(SCIF_REG_BRR, SCBRR_VALUE(in_freq, baudrate));

	set_reg(SCIF_REG_FCR, SCIF_FCR_RST_NGATE_RFTF);
	set_reg(SCIF_REG_SCR, SCIF_SCR_RCV_TRN_EN);

	sbi_console_set_device(&renesas_scif_console);

	return 0;
}

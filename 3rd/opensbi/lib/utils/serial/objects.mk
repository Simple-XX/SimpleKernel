#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

libsbiutils-objs-$(CONFIG_FDT_SERIAL) += serial/fdt_serial.o
libsbiutils-objs-$(CONFIG_FDT_SERIAL) += serial/fdt_serial_drivers.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_CADENCE) += fdt_serial_cadence
libsbiutils-objs-$(CONFIG_FDT_SERIAL_CADENCE) += serial/fdt_serial_cadence.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_GAISLER) += fdt_serial_gaisler
libsbiutils-objs-$(CONFIG_FDT_SERIAL_GAISLER) += serial/fdt_serial_gaisler.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_HTIF) += fdt_serial_htif
libsbiutils-objs-$(CONFIG_FDT_SERIAL_HTIF) += serial/fdt_serial_htif.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_RENESAS_SCIF) += fdt_serial_renesas_scif
libsbiutils-objs-$(CONFIG_FDT_SERIAL_RENESAS_SCIF) += serial/fdt_serial_renesas_scif.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_SHAKTI) += fdt_serial_shakti
libsbiutils-objs-$(CONFIG_FDT_SERIAL_SHAKTI) += serial/fdt_serial_shakti.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_SIFIVE) += fdt_serial_sifive
libsbiutils-objs-$(CONFIG_FDT_SERIAL_SIFIVE) += serial/fdt_serial_sifive.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_LITEX) += fdt_serial_litex
libsbiutils-objs-$(CONFIG_FDT_SERIAL_LITEX) += serial/fdt_serial_litex.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_UART8250) += fdt_serial_uart8250
libsbiutils-objs-$(CONFIG_FDT_SERIAL_UART8250) += serial/fdt_serial_uart8250.o

carray-fdt_serial_drivers-$(CONFIG_FDT_SERIAL_XILINX_UARTLITE) += fdt_serial_xlnx_uartlite
libsbiutils-objs-$(CONFIG_FDT_SERIAL_XILINX_UARTLITE) += serial/fdt_serial_xlnx_uartlite.o

libsbiutils-objs-$(CONFIG_SERIAL_CADENCE) += serial/cadence-uart.o
libsbiutils-objs-$(CONFIG_SERIAL_GAISLER) += serial/gaisler-uart.o
libsbiutils-objs-$(CONFIG_SERIAL_RENESAS_SCIF) += serial/renesas_scif.o
libsbiutils-objs-$(CONFIG_SERIAL_SHAKTI) += serial/shakti-uart.o
libsbiutils-objs-$(CONFIG_SERIAL_SIFIVE) += serial/sifive-uart.o
libsbiutils-objs-$(CONFIG_SERIAL_LITEX) += serial/litex-uart.o
libsbiutils-objs-$(CONFIG_SERIAL_UART8250) += serial/uart8250.o
libsbiutils-objs-$(CONFIG_SERIAL_XILINX_UARTLITE) += serial/xlnx-uartlite.o
libsbiutils-objs-$(CONFIG_SERIAL_SEMIHOSTING) += serial/semihosting.o

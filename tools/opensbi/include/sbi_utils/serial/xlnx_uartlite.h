/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Alistair Francis <alistair.francis@wdc.com>
 */
#ifndef __SERIAL_XLNX_UARTLITE_H__
#define __SERIAL_XLNX_UARTLITE_H__

#include <sbi/sbi_types.h>

int xlnx_uartlite_init(unsigned long base);

#endif

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Cobham Gaisler AB.
 *
 * Authors:
 *   Daniel Cederman <cederman@gaisler.com>
 */

#ifndef __SERIAL_GAISLER_APBUART_H__
#define __SERIAL_GAISLER_APBUART_H__

#include <sbi/sbi_types.h>

int gaisler_uart_init(unsigned long base, u32 in_freq, u32 baudrate);

#endif

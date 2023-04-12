/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 StarFive Technology Co., Ltd.
 *
 * Author: Jun Liang Tan <junliang.tan@linux.starfivetech.com>
 */

#ifndef __SERIAL_CADENCE_UART_H__
#define __SERIAL_CADENCE_UART_H__

#include <sbi/sbi_types.h>

int cadence_uart_init(unsigned long base, u32 in_freq, u32 baudrate);

#endif

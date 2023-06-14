/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2022 Renesas Electronics Corporation
 */

#ifndef __SERIAL_RENESAS_SCIF_H__
#define __SERIAL_RENESAS_SCIF_H__

int renesas_scif_init(unsigned long base, u32 in_freq, u32 baudrate);

#endif /* __SERIAL_RENESAS_SCIF_H__ */

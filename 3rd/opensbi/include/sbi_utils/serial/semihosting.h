/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <apatel@ventanamicro.com>
 *   Kautuk Consul <kconsul@ventanamicro.com>
 */

#ifndef __SERIAL_SEMIHOSTING_H__
#define __SERIAL_SEMIHOSTING_H__

#include <sbi/sbi_types.h>

/**
 * enum semihosting_open_mode - Numeric file modes for use with semihosting_open()
 * MODE_READ: 'r'
 * MODE_BINARY: 'b'
 * MODE_PLUS: '+'
 * MODE_WRITE: 'w'
 * MODE_APPEND: 'a'
 *
 * These modes represent the mode string used by fopen(3) in a form which can
 * be passed to semihosting_open(). These do NOT correspond directly to %O_RDONLY,
 * %O_CREAT, etc; see fopen(3) for details. In particular, @MODE_PLUS
 * effectively results in adding %O_RDWR, and @MODE_WRITE will add %O_TRUNC.
 * For compatibility, @MODE_BINARY should be added when opening non-text files
 * (such as images).
 */
enum semihosting_open_mode {
	MODE_READ	= 0x0,
	MODE_BINARY	= 0x1,
	MODE_PLUS	= 0x2,
	MODE_WRITE	= 0x4,
	MODE_APPEND	= 0x8,
};

#ifdef CONFIG_SERIAL_SEMIHOSTING
int semihosting_init(void);
int semihosting_enabled(void);
#else
static inline int semihosting_init(void) { return SBI_ENODEV; }
static inline int semihosting_enabled(void) { return 0; }
#endif

#endif

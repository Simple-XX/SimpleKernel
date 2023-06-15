/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2010-2020, The Regents of the University of California
 * (Regents).  All Rights Reserved.
 */

#ifndef __SYS_HTIF_H__
#define __SYS_HTIF_H__

#include <sbi/sbi_types.h>

int htif_serial_init(bool custom_addr,
		     unsigned long custom_fromhost_addr,
		     unsigned long custom_tohost_addr);

int htif_system_reset_init(bool custom_addr,
			   unsigned long custom_fromhost_addr,
			   unsigned long custom_tohost_addr);

#endif

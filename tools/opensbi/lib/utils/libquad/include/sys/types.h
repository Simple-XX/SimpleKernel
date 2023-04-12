/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Jessica Clarke <jrtc27@jrtc27.com>
 */

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <sbi/sbi_types.h>

typedef unsigned long u_long;

typedef int64_t quad_t;
typedef uint64_t u_quad_t;

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _QUAD_LOWWORD 1
#define _QUAD_HIGHWORD 0
#else
#define _QUAD_LOWWORD 0
#define _QUAD_HIGHWORD 1
#endif

#endif

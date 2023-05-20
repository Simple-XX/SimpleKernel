// SPDX-License-Identifier: BSD-2-Clause-Patent
/*
 * This code is based on EDK II MdePkg/Library/BaseLib/Math64.c
 * Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.
 */

#include "lib.h"

/**
 * LShiftU64() - left shift
 */
UINT64
LShiftU64 (
	IN UINT64   Operand,
	IN UINTN    Count
)
{
	return Operand << Count;
}

/**
 * RShiftU64() - right shift
 */
UINT64
RShiftU64 (
	IN UINT64   Operand,
	IN UINTN    Count
)
{
	return Operand >> Count;
}

/**
 * MultU64x32() - multiply
 */
UINT64
MultU64x32 (
	IN UINT64   Multiplicand,
	IN UINTN    Multiplier
)
{
	return Multiplicand * Multiplier;
}

/**
 * DivU64x32() - divide
 */
UINT64
DivU64x32 (
	IN UINT64   Dividend,
	IN UINTN    Divisor,
	OUT UINTN   *Remainder OPTIONAL
)
{
	ASSERT(Divisor != 0);

	if (Remainder) {
		*Remainder = Dividend % Divisor;
	}

	return Dividend / Divisor;
}

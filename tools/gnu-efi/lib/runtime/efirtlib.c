/*++

Copyright (c) 1999  Intel Corporation

Module Name:

    EfiRtLib.h

Abstract:

    EFI Runtime library functions



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "efirtlib.h"

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtZeroMem)
#endif
VOID
RUNTIMEFUNCTION
RtZeroMem (
    IN VOID     *Buffer,
    IN UINTN     Size
    )
{
    INT8        *pt;

    pt = Buffer;
    while (Size--) {
        *(pt++) = 0;
    }
}

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtSetMem)
#endif
VOID
RUNTIMEFUNCTION
RtSetMem (
    IN VOID     *Buffer,
    IN UINTN    Size,
    IN UINT8    Value
    )
{
    INT8        *pt;

    pt = Buffer;
    while (Size--) {
        *(pt++) = Value;
    }
}

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtCopyMem)
#endif
VOID
RUNTIMEFUNCTION
RtCopyMem (
    IN VOID        *Dest,
    IN CONST VOID  *Src,
    IN UINTN       len
    )
{
    CHAR8 *d = (CHAR8*)Dest;
    CHAR8 *s = (CHAR8*)Src;

    if (d == NULL || s == NULL || s == d)
        return;

    // If the beginning of the destination range overlaps with the end of
    // the source range, make sure to start the copy from the end so that
    // we don't end up overwriting source data that we need for the copy.
    if ((d > s) && (d < s + len)) {
        for (d += len, s += len; len--; )
            *--d = *--s;
    } else {
        while (len--)
            *d++ = *s++;
    }
}

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtCompareMem)
#endif
INTN
RUNTIMEFUNCTION
RtCompareMem (
    IN CONST VOID     *Dest,
    IN CONST VOID     *Src,
    IN UINTN    len
    )
{
    CONST CHAR8    *d = Dest, *s = Src;
    while (len--) {
        if (*d != *s) {
            return *d - *s;
        }

        d += 1;
        s += 1;
    }

    return 0;
}

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtCompareGuid)
#endif
INTN
RUNTIMEFUNCTION
RtCompareGuid (
    IN EFI_GUID     *Guid1,
    IN EFI_GUID     *Guid2
    )
/*++

Routine Description:

    Compares to GUIDs

Arguments:

    Guid1       - guid to compare
    Guid2       - guid to compare

Returns:
    = 0     if Guid1 == Guid2

--*/
{
    INT32       *g1, *g2, r;

    //
    // Compare 32 bits at a time
    //

    g1 = (INT32 *) Guid1;
    g2 = (INT32 *) Guid2;

    r  = g1[0] - g2[0];
    r |= g1[1] - g2[1];
    r |= g1[2] - g2[2];
    r |= g1[3] - g2[3];

    return r;
}



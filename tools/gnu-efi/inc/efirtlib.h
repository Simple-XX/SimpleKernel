#ifndef _EFI_RT_LIB_INCLUDE_
#define _EFI_RT_LIB_INCLUDE_
/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    efilib.h

Abstract:

    EFI Runtime library functions



Revision History

--*/

#include "efidebug.h"
#include "efipart.h"
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#include "x86_64/efilibplat.h"
#elif defined(_M_IX86) || defined(__i386__)
#include "ia32/efilibplat.h"
#elif defined(_M_IA64) || defined(__ia64__)
#include "ia64/efilibplat.h"
#elif defined (_M_ARM64) || defined(__aarch64__)
#include "aarch64/efilibplat.h"
#elif defined (_M_ARM) || defined(__arm__)
#include "arm/efilibplat.h"
#elif defined (_M_MIPS64) || defined(__mips64__) || defined(__mips64)
#include "mips64el/efilibplat.h"
#elif defined (__riscv) && __riscv_xlen == 64
#include "riscv64/efilibplat.h"
#endif


VOID
RUNTIMEFUNCTION
RtZeroMem (
    IN VOID     *Buffer,
    IN UINTN     Size
    );

VOID
RUNTIMEFUNCTION
RtSetMem (
    IN VOID     *Buffer,
    IN UINTN    Size,
    IN UINT8    Value    
    );

VOID
RUNTIMEFUNCTION
RtCopyMem (
    IN VOID     *Dest,
    IN CONST VOID     *Src,
    IN UINTN    len
    );

INTN
RUNTIMEFUNCTION
RtCompareMem (
    IN CONST VOID     *Dest,
    IN CONST VOID     *Src,
    IN UINTN    len
    );

INTN
RUNTIMEFUNCTION
RtStrCmp (
    IN CONST CHAR16   *s1,
    IN CONST CHAR16   *s2
    );


VOID
RUNTIMEFUNCTION
RtStrCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16    *Src
    );

VOID
RUNTIMEFUNCTION
RtStrnCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16    *Src,
    IN UINTN     Len
    );

CHAR16 *
RUNTIMEFUNCTION
RtStpCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16    *Src
    );

CHAR16 *
RUNTIMEFUNCTION
RtStpnCpy (
    IN CHAR16   *Dest,
    IN CONST CHAR16    *Src,
    IN UINTN     Len
    );

VOID
RUNTIMEFUNCTION
RtStrCat (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src
    );

VOID
RUNTIMEFUNCTION
RtStrnCat (
    IN CHAR16   *Dest,
    IN CONST CHAR16   *Src,
    IN UINTN     Len
    );

UINTN
RUNTIMEFUNCTION
RtStrLen (
    IN CONST CHAR16   *s1
    );

UINTN
RUNTIMEFUNCTION
RtStrnLen (
    IN CONST CHAR16   *s1,
    IN UINTN           Len
    );

UINTN
RUNTIMEFUNCTION
RtStrSize (
    IN CONST CHAR16   *s1
    );

INTN
RUNTIMEFUNCTION
RtCompareGuid (
    IN EFI_GUID     *Guid1,
    IN EFI_GUID     *Guid2
    );

UINT8
RUNTIMEFUNCTION
RtDecimaltoBCD(
    IN  UINT8 BcdValue
    );

UINT8
RUNTIMEFUNCTION
RtBCDtoDecimal(
    IN  UINT8 BcdValue
    );

//
// Virtual mapping transition support.  (Only used during
// the virtual address change transisition)
//

VOID
RUNTIMEFUNCTION
RtLibEnableVirtualMappings (
    VOID
    );

VOID
RUNTIMEFUNCTION
RtConvertList (
    IN UINTN            DebugDisposition,
    IN OUT LIST_ENTRY   *ListHead
    );

VOID
RUNTIMEFUNCTION
RtAcquireLock (
    IN FLOCK    *Lock
    );

VOID
RUNTIMEFUNCTION
RtReleaseLock (
    IN FLOCK    *Lock
    );


#endif

/*++

Copyright (c) 1999  Intel Corporation
    
Module Name:

    salpal.c

Abstract:

    Functions to make SAL and PAL proc calls

Revision History

--*/
#include "lib.h"
#include "palproc.h"
#include "salproc.h"
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

rArg
MakeStaticPALCall (
    IN UINT64   PALPROCPtr,
    IN UINT64   Arg1,
    IN UINT64   Arg2,
    IN UINT64   Arg3,
    IN UINT64   Arg4
    );

rArg
MakeStackedPALCall (
    IN UINT64   PALPROCPtr,
    IN UINT64   Arg1,
    IN UINT64   Arg2,
    IN UINT64   Arg3,
    IN UINT64   Arg4
    );


PLABEL   SalProcPlabel;
PLABEL   PalProcPlabel;
CALL_SAL_PROC   GlobalSalProc;
CALL_PAL_PROC   GlobalPalProc;

VOID
LibInitSalAndPalProc (
    OUT PLABEL  *SalPlabel,
    OUT UINT64  *PalEntry
    )
{
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    EFI_STATUS                          Status;

    GlobalSalProc = NULL;
    GlobalPalProc = NULL;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, (VOID **)&SalSystemTable);
    if (EFI_ERROR(Status)) {
        return; 
    }

    //
    // BugBug: Add code to test checksum on the Sal System Table
    //
    if (SalSystemTable->Entry0.Type != 0) {
        return;
    }

    SalProcPlabel.ProcEntryPoint = SalSystemTable->Entry0.SalProcEntry; 
    SalProcPlabel.GP             = SalSystemTable->Entry0.GlobalDataPointer;
    GlobalSalProc                = (CALL_SAL_PROC)&SalProcPlabel.ProcEntryPoint;

    //
    // Need to check the PAL spec to make sure I'm not responsible for
    //  storing more state.
    // We are passing in a Plabel that should be ignorred by the PAL. Call
    //  this way will cause use to retore our gp after the PAL returns.
    //
    PalProcPlabel.ProcEntryPoint = SalSystemTable->Entry0.PalProcEntry; 
    PalProcPlabel.GP             = SalSystemTable->Entry0.GlobalDataPointer;
    GlobalPalProc                = (CALL_PAL_PROC)PalProcPlabel.ProcEntryPoint;

    *PalEntry = PalProcPlabel.ProcEntryPoint;
    *SalPlabel = SalProcPlabel;
}

EFI_STATUS
LibGetSalIoPortMapping (
    OUT UINT64  *IoPortMapping
    )
/*++

  Get the IO Port Map from the SAL System Table.
  DO NOT USE THIS TO DO YOU OWN IO's!!!!!!!!!!!!
  Only use this for getting info, or initing the built in EFI IO abstraction.
  Always use the EFI Device IO protoocl to access IO space.
  
--*/
{
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    SAL_ST_MEMORY_DESCRIPTOR_ENTRY      *SalMemDesc;
    EFI_STATUS                          Status;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, (VOID **)&SalSystemTable);
    if (EFI_ERROR(Status)) {
        return EFI_UNSUPPORTED; 
    }

    //
    // BugBug: Add code to test checksum on the Sal System Table
    //
    if (SalSystemTable->Entry0.Type != 0) {
        return EFI_UNSUPPORTED;
    }

    //
    // The SalSystemTable pointer includes the Type 0 entry.
    //  The SalMemDesc is Type 1 so it comes next.
    //
    SalMemDesc = (SAL_ST_MEMORY_DESCRIPTOR_ENTRY *)(SalSystemTable + 1);
    while (SalMemDesc->Type == SAL_ST_MEMORY_DESCRIPTOR) {
        if (SalMemDesc->MemoryType == SAL_IO_PORT_MAPPING) {
            *IoPortMapping = SalMemDesc->PhysicalMemoryAddress;
            return EFI_SUCCESS;
        }
        SalMemDesc++;
   } 
    return EFI_UNSUPPORTED;
}

EFI_STATUS
LibGetSalIpiBlock (
    OUT UINT64  *IpiBlock
    )
/*++

  Get the IPI block from the SAL system table
  
--*/
{
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    SAL_ST_MEMORY_DESCRIPTOR_ENTRY      *SalMemDesc;
    EFI_STATUS                          Status;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, (VOID*)&SalSystemTable);
    if (EFI_ERROR(Status)) {
        return EFI_UNSUPPORTED; 
    }

    //
    // BugBug: Add code to test checksum on the Sal System Table
    //
    if (SalSystemTable->Entry0.Type != 0) {
        return EFI_UNSUPPORTED;
    }

    //
    // The SalSystemTable pointer includes the Type 0 entry.
    //  The SalMemDesc is Type 1 so it comes next.
    //
    SalMemDesc = (SAL_ST_MEMORY_DESCRIPTOR_ENTRY *)(SalSystemTable + 1);
    while (SalMemDesc->Type == SAL_ST_MEMORY_DESCRIPTOR) {
        if (SalMemDesc->MemoryType == SAL_SAPIC_IPI_BLOCK ) {
            *IpiBlock = SalMemDesc->PhysicalMemoryAddress;
            return EFI_SUCCESS;
        }
        SalMemDesc++;
    }
    return EFI_UNSUPPORTED;
}

EFI_STATUS
LibGetSalWakeupVector (
    OUT UINT64  *WakeVector
    )
/*++

Get the wakeup vector from the SAL system table
  
--*/
{
    SAL_ST_AP_WAKEUP_DECRIPTOR      *ApWakeUp;

    ApWakeUp = LibSearchSalSystemTable (SAL_ST_AP_WAKEUP);
    if (!ApWakeUp) {
        *WakeVector = -1;
        return EFI_UNSUPPORTED;
    }
    *WakeVector = ApWakeUp->ExternalInterruptVector;
    return EFI_SUCCESS;
}

VOID *
LibSearchSalSystemTable (
    IN  UINT8   EntryType  
    )
{
    EFI_STATUS                          Status;
    UINT8                               *SalTableHack;
    SAL_SYSTEM_TABLE_ASCENDING_ORDER    *SalSystemTable;
    UINT16                              EntryCount;
    UINT16                              Count;

    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid, (VOID*)&SalSystemTable);
    if (EFI_ERROR(Status)) {
        return NULL; 
    }

    EntryCount = SalSystemTable->Header.EntryCount;
    if (EntryCount == 0) {
        return NULL;
    }
    //
    // BugBug: Add code to test checksum on the Sal System Table
    //

    SalTableHack = (UINT8 *)&SalSystemTable->Entry0;
    for (Count = 0; Count < EntryCount ;Count++) {
        if (*SalTableHack == EntryType) {
            return (VOID *)SalTableHack;
        }
        switch (*SalTableHack) {
        case SAL_ST_ENTRY_POINT:
            SalTableHack += 48;
            break;
        case SAL_ST_MEMORY_DESCRIPTOR:
            SalTableHack += 32;
            break;
        case SAL_ST_PLATFORM_FEATURES:
            SalTableHack += 16;
            break;
        case SAL_ST_TR_USAGE:
            SalTableHack += 32;
            break;
        case SAL_ST_PTC:
            SalTableHack += 16;
            break;
        case SAL_ST_AP_WAKEUP:
            SalTableHack += 16;
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
    return NULL;
}

VOID
LibSalProc (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    )
{
    rArg    ReturnValue;

    ReturnValue.p0 = -3;    // SAL status return completed with error 
    if (GlobalSalProc) {
        ReturnValue = GlobalSalProc(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    }

    if (Results) {
        CopyMem (Results, &ReturnValue, sizeof(rArg));
    }
}

VOID
LibPalProc (
    IN  UINT64    Arg1, // Pal Proc index
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    OUT rArg      *Results  OPTIONAL
    )
{
    
    rArg    ReturnValue;

    ReturnValue.p0 = -3;    // PAL status return completed with error 

    //
    // check for valid PalProc entry point
    //
    
    if (!GlobalPalProc) {
        if (Results) 
            CopyMem (Results, &ReturnValue, sizeof(rArg));
        return;
    }
        
    //
    // check if index falls within stacked or static register calling conventions
    // and call appropriate Pal stub call
    //

    if (((Arg1 >=255) && (Arg1 <=511)) ||
        ((Arg1 >=768) && (Arg1 <=1023))) {    
            ReturnValue = MakeStackedPALCall((UINT64)GlobalPalProc,Arg1,Arg2,Arg3,Arg4);
    }
    else {
        ReturnValue = MakeStaticPALCall((UINT64)GlobalPalProc,Arg1,Arg2,Arg3,Arg4);
    }
          
    if (Results) 
        CopyMem (Results, &ReturnValue, sizeof(rArg));
        
    return;
}


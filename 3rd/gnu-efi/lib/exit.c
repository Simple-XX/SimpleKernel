#include "lib.h"

VOID
Exit(
    IN EFI_STATUS   ExitStatus,
    IN UINTN        ExitDataSize,
    IN CHAR16       *ExitData OPTIONAL
    )
{
    uefi_call_wrapper(BS->Exit,
            4,
            LibImageHandle,
            ExitStatus,
            ExitDataSize,
            ExitData);

    // Uh oh, Exit() returned?!
    for (;;) { }
}

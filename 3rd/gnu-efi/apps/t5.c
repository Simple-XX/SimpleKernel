#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	InitializeLib(image, systab);
	Print(L"HelloLib application started\n");
	Print(L"\n\n\nHit any key to exit this image\n");
	WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
	uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"\n\n");
	return EFI_SUCCESS;
}

#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main (EFI_HANDLE *image, EFI_SYSTEM_TABLE *systab)
{
	UINTN index;

        InitializeLib(image, systab);
	uefi_call_wrapper(systab->ConOut->OutputString, 2, systab->ConOut, L"Hello application started\r\n");
	uefi_call_wrapper(systab->ConOut->OutputString, 2, systab->ConOut, L"\r\n\r\n\r\nHit any key to exit\r\n");
	uefi_call_wrapper(systab->BootServices->WaitForEvent, 3, 1, &systab->ConIn->WaitForKey, &index);
	return EFI_SUCCESS;
}


#include <efi.h>
#include <efilib.h>
#include <efisetjmp.h>

EFI_STATUS
efi_main(
	EFI_HANDLE image_handle,
	EFI_SYSTEM_TABLE *systab
)
{
	jmp_buf env;
	int rc;

	InitializeLib(image_handle, systab);
	rc = setjmp(env);
	Print(L"setjmp() = %d\n", rc);

	if (rc == 3) {
		Print(L"3 worked\n");
		longjmp(env, 0);
		return 0;
	}

	if (rc == 1) {
		Print(L"0 got to be one yay\n");
		return 0;
	}

	longjmp(env, 3);
	return 0;
}

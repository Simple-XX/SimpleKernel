/*
 * ctors.c
 * Copyright 2019 Peter Jones <pjones@redhat.com>
 *
 */

#include <efi.h>
#include <efilib.h>

/*
 * Note that these aren't the using the GNU "CONSTRUCTOR" output section
 * command, so they don't start with a size.  Because of p2align and the
 * end/END definitions, and the fact that they're mergeable, they can also
 * have NULLs which aren't guaranteed to be at the end.
 */
extern UINTN _init_array, _init_array_end;
extern UINTN __CTOR_LIST__, __CTOR_END__;
extern UINTN _fini_array, _fini_array_end;
extern UINTN __DTOR_LIST__, __DTOR_END__;

typedef void (*funcp)(void);

static void ctors(void)
{
	for (funcp *location = (void *)&_init_array; location < (funcp *)&_init_array_end; location++) {
		funcp func = *location;
		if (location != NULL)
			func();
	}

	for (funcp *location = (void *)&__CTOR_LIST__; location < (funcp *)&__CTOR_END__; location++) {
		funcp func = *location;
		if (location != NULL)
			func();
	}
}

static void dtors(void)
{
	for (funcp *location = (void *)&__DTOR_LIST__; location < (funcp *)&__DTOR_END__; location++) {
		funcp func = *location;
		if (location != NULL)
			func();
	}

	for (funcp *location = (void *)&_fini_array; location < (funcp *)&_fini_array_end; location++) {
		funcp func = *location;
		if (location != NULL)
			func();
	}
}

extern EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab);

EFI_STATUS _entry(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	EFI_STATUS status;
	InitializeLib(image, systab);

	ctors();
	status = efi_main(image, systab);
	dtors();

	return status;
}

// vim:fenc=utf-8:tw=75:noet

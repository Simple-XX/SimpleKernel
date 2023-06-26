/*
 * ctors_test.c
 * Copyright 2019 Peter Jones <pjones@redhat.com>
 *
 */

#include <efi.h>
#include <efilib.h>

extern int constructed_value;

EFI_STATUS
efi_main (EFI_HANDLE image EFI_UNUSED, EFI_SYSTEM_TABLE *systab EFI_UNUSED)
{
	Print(L"%a:%d:%a() constructed_value:%d\n", __FILE__, __LINE__, __func__, constructed_value);

	return EFI_SUCCESS;
}

// vim:fenc=utf-8:tw=75:noet

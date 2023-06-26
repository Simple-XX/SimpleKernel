/*
 * ctors.c
 * Copyright 2019 Peter Jones <pjones@redhat.com>
 *
 */

#include <efi.h>
#include <efilib.h>

int constructed_value = 0;

static void __attribute__((__constructor__)) ctor(void)
{
	Print(L"%a:%d:%a() constructed_value:%d\n", __FILE__, __LINE__, __func__, constructed_value);
	constructed_value = 1;
	Print(L"%a:%d:%a() constructed_value:%d\n", __FILE__, __LINE__, __func__, constructed_value);
}

static void __attribute__((__destructor__)) dtor(void)
{
	Print(L"%a:%d:%a() constructed_value:%d\n", __FILE__, __LINE__, __func__, constructed_value);
	constructed_value = 0;
	Print(L"%a:%d:%a() constructed_value:%d\n", __FILE__, __LINE__, __func__, constructed_value);
}

// vim:fenc=utf-8:tw=75:noet

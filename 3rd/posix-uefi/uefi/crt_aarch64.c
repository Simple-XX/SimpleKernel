/*
 * crt_aarch64.c
 *
 * Copyright (C) 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the POSIX-UEFI package.
 * @brief C runtime, bootstraps an EFI application to call standard main()
 *
 */

#include <uefi.h>

/* this is implemented by the application */
extern int main(int argc, char_t **argv);

/* definitions for elf relocations */
#ifndef __clang__
typedef uint64_t Elf64_Xword;
typedef	int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef struct
{
  Elf64_Sxword  d_tag;              /* Dynamic entry type */
  union
    {
      Elf64_Xword d_val;            /* Integer value */
      Elf64_Addr d_ptr;             /* Address value */
    } d_un;
} Elf64_Dyn;
#define DT_NULL             0       /* Marks end of dynamic section */
#define DT_RELA             7       /* Address of Rela relocs */
#define DT_RELASZ           8       /* Total size of Rela relocs */
#define DT_RELAENT          9       /* Size of one Rela reloc */
typedef struct
{
  Elf64_Addr    r_offset;           /* Address */
  Elf64_Xword   r_info;             /* Relocation type and symbol index */
} Elf64_Rel;
#define ELF64_R_TYPE(i)     ((i) & 0xffffffff)
#define R_AARCH64_RELATIVE  1027    /* Adjust by program base */
#endif

/* globals to store system table pointers */
efi_handle_t IM = NULL;
efi_system_table_t *ST = NULL;
efi_boot_services_t *BS = NULL;
efi_runtime_services_t *RT = NULL;
efi_loaded_image_protocol_t *LIP = NULL;
#ifndef UEFI_NO_UTF8
char *__argvutf8 = NULL;
#endif

/* we only need one .o file, so use inline Assembly here */
void bootstrap(void)
{
    __asm__ __volatile__ (
    /* call init in C */
    "	.align 4\n"
#ifndef __clang__
    "	.globl _start\n"
    "_start:\n"
    "	ldr		x2, =ImageBase\n"
    "	adrp	x3, _DYNAMIC\n"
    "	add		x3, x3, #:lo12:_DYNAMIC\n"
    "	bl		uefi_init\n"
    "	ret\n"

    /* fake a relocation record, so that EFI won't complain */
    " 	.data\n"
    "dummy:	.long	0\n"
    " 	.section .reloc, \"a\"\n"
    "label1:\n"
    "	.long	dummy-label1\n"
    " 	.long	10\n"
    "    .word 0\n"
    ".text\n"
#else
    "	.globl __chkstk\n"
    "__chkstk:\n"
    "	ret\n"
#endif
    );

    /* setjmp and longjmp */
    __asm__ __volatile__ (
    "	.p2align 3\n"
    "	.globl	setjmp\n"
    "setjmp:\n"
    "	mov	x16, sp\n"
    "	stp	x19, x20, [x0, #0]\n"
    "	stp	x21, x22, [x0, #16]\n"
    "	stp	x23, x24, [x0, #32]\n"
    "	stp	x25, x26, [x0, #48]\n"
    "	stp	x27, x28, [x0, #64]\n"
    "	stp	x29, x30, [x0, #80]\n"
    "	str	x16, [x0, #96]\n"
    "	stp	d8, d9, [x0, #112]\n"
    "	stp	d10, d11, [x0, #128]\n"
    "	stp	d12, d13, [x0, #144]\n"
    "	stp	d14, d15, [x0, #160]\n"
    "	mov	w0, #0\n"
    "	ret\n"
    );
    __asm__ __volatile__ (
    "	.globl	longjmp\n"
    "longjmp:\n"
    "	ldp	x19, x20, [x0, #0]\n"
    "	ldp	x21, x22, [x0, #16]\n"
    "	ldp	x23, x24, [x0, #32]\n"
    "	ldp	x25, x26, [x0, #48]\n"
    "	ldp	x27, x28, [x0, #64]\n"
    "	ldp	x29, x30, [x0, #80]\n"
    "	ldr	x16, [x0, #96]\n"
    "	ldp	d8, d9, [x0, #112]\n"
    "	ldp	d10, d11, [x0, #128]\n"
    "	ldp	d12, d13, [x0, #144]\n"
    "	ldp	d14, d15, [x0, #160]\n"
    "	mov	sp, x16\n"
    "	cmp	w1, #0\n"
    "	mov	w0, #1\n"
    "	csel	w0, w1, w0, ne\n"
    "	br	x30\n"
    );
}

/**
 * Initialize POSIX-UEFI and call the application's main() function
 */
efi_status_t uefi_init (
    efi_handle_t image, efi_system_table_t *systab
#ifndef __clang__
    , uintptr_t ldbase, Elf64_Dyn *dyn
#endif
) {
    efi_guid_t shpGuid = EFI_SHELL_PARAMETERS_PROTOCOL_GUID;
    efi_shell_parameters_protocol_t *shp = NULL;
    efi_guid_t shiGuid = SHELL_INTERFACE_PROTOCOL_GUID;
    efi_shell_interface_protocol_t *shi = NULL;
    efi_guid_t lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    efi_status_t status;
    int argc = 0, i, ret;
    wchar_t **argv = NULL;
#ifndef UEFI_NO_UTF8
    int j;
    char *s;
#endif
#ifndef __clang__
    long relsz = 0, relent = 0;
    Elf64_Rel *rel = 0;
    uintptr_t *addr;
    /* handle relocations */
    for (i = 0; dyn[i].d_tag != DT_NULL; ++i) {
        switch (dyn[i].d_tag) {
            case DT_RELA: rel = (Elf64_Rel*)((unsigned long)dyn[i].d_un.d_ptr + ldbase); break;
            case DT_RELASZ: relsz = dyn[i].d_un.d_val; break;
            case DT_RELAENT: relent = dyn[i].d_un.d_val; break;
            default: break;
        }
    }
    if (rel && relent) {
        while (relsz > 0) {
            if(ELF64_R_TYPE (rel->r_info) == R_AARCH64_RELATIVE)
                { addr = (unsigned long *)(ldbase + rel->r_offset); *addr += ldbase; }
            rel = (Elf64_Rel*) ((char *) rel + relent);
            relsz -= relent;
        }
    }
#else
    (void)i;
#endif
    /* failsafes, should never happen */
    if(!image || !systab || !systab->BootServices || !systab->BootServices->HandleProtocol ||
        !systab->BootServices->OpenProtocol || !systab->BootServices->AllocatePool || !systab->BootServices->FreePool)
            return EFI_UNSUPPORTED;
    /* save EFI pointers and loaded image into globals */
    IM = image;
    ST = systab;
    BS = systab->BootServices;
    RT = systab->RuntimeServices;
    BS->HandleProtocol(image, &lipGuid, (void **)&LIP);
    /* get command line arguments */
    status = BS->OpenProtocol(image, &shpGuid, (void **)&shp, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(!EFI_ERROR(status) && shp) { argc = (int)shp->Argc; argv = shp->Argv; }
    else {
        /* if shell 2.0 failed, fallback to shell 1.0 interface */
        status = BS->OpenProtocol(image, &shiGuid, (void **)&shi, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if(!EFI_ERROR(status) && shi) { argc = (int)shi->Argc; argv = shi->Argv; }
    }
    /* call main */
#ifndef UEFI_NO_UTF8
    if(argc && argv) {
        ret = (argc + 1) * ((int)sizeof(uintptr_t) + 1);
        for(i = 0; i < argc; i++)
            for(j = 0; argv[i] && argv[i][j]; j++)
                ret += argv[i][j] < 0x80 ? 1 : (argv[i][j] < 0x800 ? 2 : 3);
        status = BS->AllocatePool(LIP ? LIP->ImageDataType : EfiLoaderData, (uintn_t)ret, (void **)&__argvutf8);
        if(EFI_ERROR(status) || !__argvutf8) { argc = 0; __argvutf8 = NULL; }
        else {
            s = __argvutf8 + argc * (int)sizeof(uintptr_t);
            *((uintptr_t*)s) = (uintptr_t)0; s += sizeof(uintptr_t);
            for(i = 0; i < argc; i++) {
                *((uintptr_t*)(__argvutf8 + i * (int)sizeof(uintptr_t))) = (uintptr_t)s;
                for(j = 0; argv[i] && argv[i][j]; j++) {
                    if(argv[i][j]<0x80) { *s++ = argv[i][j]; } else
                    if(argv[i][j]<0x800) { *s++ = ((argv[i][j]>>6)&0x1F)|0xC0; *s++ = (argv[i][j]&0x3F)|0x80; } else
                    { *s++ = ((argv[i][j]>>12)&0x0F)|0xE0; *s++ = ((argv[i][j]>>6)&0x3F)|0x80; *s++ = (argv[i][j]&0x3F)|0x80; }
                }
                *s++ = 0;
            }
        }
    }
    ret = main(argc, (char**)__argvutf8);
    if(__argvutf8) BS->FreePool(__argvutf8);
    return ret;
#else
    ret = main(argc, argv);
#endif
    return ret ? EFIERR(ret) : EFI_SUCCESS;
}

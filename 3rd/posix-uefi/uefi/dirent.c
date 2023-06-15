/*
 * dirent.c
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
 * @brief Implementing functions which are defined in dirent.h
 *
 */

#include <uefi.h>

extern void __stdio_seterrno(efi_status_t status);
static struct dirent __dirent;

DIR *opendir (const char_t *__name)
{
    DIR *dp = (DIR*)fopen(__name, CL("rd"));
    if(dp) rewinddir(dp);
    return dp;
}

struct dirent *readdir (DIR *__dirp)
{
    efi_status_t status;
    efi_file_info_t info;
    uintn_t bs = sizeof(efi_file_info_t);
    memset(&__dirent, 0, sizeof(struct dirent));
    status = __dirp->Read(__dirp, &bs, &info);
    if(EFI_ERROR(status) || !bs) {
        if(EFI_ERROR(status)) __stdio_seterrno(status);
        else errno = 0;
        return NULL;
    }
    __dirent.d_type = info.Attribute & EFI_FILE_DIRECTORY ? DT_DIR : DT_REG;
#ifndef UEFI_NO_UTF8
    __dirent.d_reclen = (unsigned short int)wcstombs(__dirent.d_name, info.FileName, FILENAME_MAX - 1);
#else
    __dirent.d_reclen = strlen(info.FileName);
    strncpy(__dirent.d_name, info.FileName, FILENAME_MAX - 1);
#endif
    return &__dirent;
}

void rewinddir (DIR *__dirp)
{
    if(__dirp)
        __dirp->SetPosition(__dirp, 0);
}

int closedir (DIR *__dirp)
{
    return fclose((FILE*)__dirp);
}



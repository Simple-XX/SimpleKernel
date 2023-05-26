/*
 * unistd.c
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
 * @brief Implementing functions which are defined in unistd.h
 *
 */

#include <uefi.h>

int __remove(const wchar_t *__filename, int isdir);

int usleep (unsigned long int __useconds)
{
    BS->Stall(__useconds);
    return 0;
}

unsigned int sleep (unsigned int __seconds)
{
    BS->Stall((unsigned long int)__seconds * 1000000UL);
    return 0;
}

int unlink (const wchar_t *__filename)
{
    return __remove(__filename, 0);
}

int rmdir (const wchar_t *__filename)
{
    return __remove(__filename, 1);
}

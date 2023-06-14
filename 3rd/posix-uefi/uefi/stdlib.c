/*
 * stdlib.c
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
 * @brief Implementing functions which are defined in stdlib.h
 *
 */

#include <uefi.h>

int errno = 0;
static uint64_t __srand_seed = 6364136223846793005ULL;
extern void __stdio_cleanup(void);
#ifndef UEFI_NO_TRACK_ALLOC
static uintptr_t *__stdlib_allocs = NULL;
static uintn_t __stdlib_numallocs = 0;
#endif

int atoi(const char_t *s)
{
    return (int)atol(s);
}

int64_t atol(const char_t *s)
{
    int64_t sign = 1;
    if(!s || !*s) return 0;
    if(*s == CL('-')) { sign = -1; s++; }
    if(s[0] == CL('0')) {
        if(s[1] == CL('x'))
            return strtol(s + 2, NULL, 16) * sign;
        if(s[1] >= CL('0') && s[1] <= CL('7'))
            return strtol(s, NULL, 8) * sign;
    }
    return strtol(s, NULL, 10) * sign;
}

int64_t strtol (const char_t *s, char_t **__endptr, int __base)
{
    int64_t v=0, sign = 1;
    if(!s || !*s) return 0;
    if(*s == CL('-')) { sign = -1; s++; }
    while(!(*s < CL('0') || (__base < 10 && *s >= __base + CL('0')) || (__base >= 10 && ((*s > CL('9') && *s < CL('A')) ||
            (*s > CL('F') && *s < CL('a')) || *s > CL('f'))))) {
        v *= __base;
        if(*s >= CL('0') && *s <= (__base < 10 ? __base + CL('0') : CL('9')))
            v += (*s)-CL('0');
        else if(__base == 16 && *s >= CL('a') && *s <= CL('f'))
            v += (*s)-CL('a')+10;
        else if(__base == 16 && *s >= CL('A') && *s <= CL('F'))
            v += (*s)-CL('A')+10;
        s++;
    }
    if(__endptr) *__endptr = (char_t*)s;
    return v * sign;
}

void *malloc (size_t __size)
{
    void *ret = NULL;
    efi_status_t status;
#ifndef UEFI_NO_TRACK_ALLOC
    uintn_t i;
    for(i = 0; i < __stdlib_numallocs && __stdlib_allocs[i] != 0; i += 2);
    if(i == __stdlib_numallocs) {
        /* no free slots found, (re)allocate the housekeeping array */
        status = BS->AllocatePool(LIP ? LIP->ImageDataType : EfiLoaderData, (__stdlib_numallocs + 2) * sizeof(uintptr_t), &ret);
        if(EFI_ERROR(status) || !ret) { errno = ENOMEM; return NULL; }
        if(__stdlib_allocs) memcpy(ret, __stdlib_allocs, __stdlib_numallocs * sizeof(uintptr_t));
        __stdlib_allocs = (uintptr_t*)ret;
        __stdlib_allocs[i] = __stdlib_allocs[i + 1] = 0;
        __stdlib_numallocs += 2;
        ret = NULL;
    }
#endif
    status = BS->AllocatePool(LIP ? LIP->ImageDataType : EfiLoaderData, __size, &ret);
    if(EFI_ERROR(status) || !ret) { errno = ENOMEM; ret = NULL; }
#ifndef UEFI_NO_TRACK_ALLOC
    __stdlib_allocs[i] = (uintptr_t)ret;
    __stdlib_allocs[i + 1] = (uintptr_t)__size;
#endif
    return ret;
}

void *calloc (size_t __nmemb, size_t __size)
{
    void *ret = malloc(__nmemb * __size);
    if(ret) memset(ret, 0, __nmemb * __size);
    return ret;
}

void *realloc (void *__ptr, size_t __size)
{
    void *ret = NULL;
    efi_status_t status;
#ifndef UEFI_NO_TRACK_ALLOC
    uintn_t i;
#endif
    if(!__ptr) return malloc(__size);
    if(!__size) { free(__ptr); return NULL; }
#ifndef UEFI_NO_TRACK_ALLOC
    /* get the slot which stores the old size for this buffer */
    for(i = 0; i < __stdlib_numallocs && __stdlib_allocs[i] != (uintptr_t)__ptr; i += 2);
    if(i == __stdlib_numallocs) { errno = ENOMEM; return NULL; }
    /* allocate a new buffer and copy data from old buffer */
    status = BS->AllocatePool(LIP ? LIP->ImageDataType : EfiLoaderData, __size, &ret);
    if(EFI_ERROR(status) || !ret) { errno = ENOMEM; ret = NULL; }
    else {
        memcpy(ret, (void*)__stdlib_allocs[i], __stdlib_allocs[i + 1] < __size ? __stdlib_allocs[i + 1] : __size);
        if(__size > __stdlib_allocs[i + 1]) memset((uint8_t*)ret + __stdlib_allocs[i + 1], 0, __size - __stdlib_allocs[i + 1]);
        /* free old buffer and store new buffer in slot */
        BS->FreePool((void*)__stdlib_allocs[i]);
        __stdlib_allocs[i] = (uintptr_t)ret;
        __stdlib_allocs[i + 1] = (uintptr_t)__size;
    }
#else
    status = BS->AllocatePool(LIP ? LIP->ImageDataType : EfiLoaderData, __size, &ret);
    if(EFI_ERROR(status) || !ret) { errno = ENOMEM; return NULL; }
    /* this means out of bounds read, but fine with POSIX as the end of new buffer supposed to be left uninitialized) */
    memcpy(ret, (void*)__ptr, __size);
    BS->FreePool((void*)__ptr);
#endif
    return ret;
}

void free (void *__ptr)
{
    efi_status_t status;
#ifndef UEFI_NO_TRACK_ALLOC
    uintn_t i;
#endif
    if(!__ptr) { errno = ENOMEM; return; }
#ifndef UEFI_NO_TRACK_ALLOC
    /* find and clear the slot */
    for(i = 0; i < __stdlib_numallocs && __stdlib_allocs[i] != (uintptr_t)__ptr; i += 2);
    if(i == __stdlib_numallocs) { errno = ENOMEM; return; }
    __stdlib_allocs[i] = 0;
    __stdlib_allocs[i + 1] = 0;
    /* if there are only empty slots, free the housekeeping array too */
    for(i = 0; i < __stdlib_numallocs && __stdlib_allocs[i] == 0; i += 2);
    if(i == __stdlib_numallocs) { BS->FreePool(__stdlib_allocs); __stdlib_allocs = NULL; __stdlib_numallocs = 0; }
#endif
    status = BS->FreePool(__ptr);
    if(EFI_ERROR(status)) errno = ENOMEM;
}

void abort ()
{
#ifndef UEFI_NO_TRACK_ALLOC
    if(__stdlib_allocs)
        BS->FreePool(__stdlib_allocs);
    __stdlib_allocs = NULL;
    __stdlib_numallocs = 0;
#endif
    __stdio_cleanup();
    BS->Exit(IM, EFI_ABORTED, 0, NULL);
}

void exit (int __status)
{
#ifndef UEFI_NO_TRACK_ALLOC
    if(__stdlib_allocs)
        BS->FreePool(__stdlib_allocs);
    __stdlib_allocs = NULL;
    __stdlib_numallocs = 0;
#endif
    __stdio_cleanup();
    BS->Exit(IM, !__status ? 0 : (__status < 0 ? EFIERR(-__status) : EFIERR(__status)), 0, NULL);
}

int exit_bs()
{
    efi_status_t status = 0;
    efi_memory_descriptor_t *memory_map = NULL;
    uintn_t cnt = 3, memory_map_size=0, map_key=0, desc_size=0;
#ifndef UEFI_NO_TRACK_ALLOC
    if(__stdlib_allocs)
        BS->FreePool(__stdlib_allocs);
    __stdlib_allocs = NULL;
    __stdlib_numallocs = 0;
#endif
    __stdio_cleanup();
    while(cnt--) {
        status = BS->GetMemoryMap(&memory_map_size, memory_map, &map_key, &desc_size, NULL);
        if (status!=EFI_BUFFER_TOO_SMALL) break;
        status = BS->ExitBootServices(IM, map_key);
        if(!EFI_ERROR(status)) return 0;
    }
    return (int)(status & 0xffff);
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, __compar_fn_t cmp)
{
    uint64_t s=0, e=nmemb, m;
    int ret;
    while (s < e) {
        m = s + (e-s)/2;
        ret = cmp(key, (uint8_t*)base + m*size);
        if (ret < 0) e = m; else
        if (ret > 0) s = m+1; else
            return (void *)((uint8_t*)base + m*size);
    }
    return NULL;
}

int mblen(const char *s, size_t n)
{
    const char *e = s+n;
    int c = 0;
    if(s) {
        while(s < e && *s) {
            if((*s & 128) != 0) {
                if((*s & 32) == 0 ) s++; else
                if((*s & 16) == 0 ) s+=2; else
                if((*s & 8) == 0 ) s+=3;
            }
            c++;
            s++;
        }
    }
    return c;
}

int mbtowc (wchar_t * __pwc, const char *s, size_t n)
{
    wchar_t arg;
    int ret = 1;
    if(!s || !*s) return 0;
    arg = (wchar_t)*s;
    if((*s & 128) != 0) {
        if((*s & 32) == 0 && n > 0) { arg = ((*s & 0x1F)<<6)|(*(s+1) & 0x3F); ret = 2; } else
        if((*s & 16) == 0 && n > 1) { arg = ((*s & 0xF)<<12)|((*(s+1) & 0x3F)<<6)|(*(s+2) & 0x3F); ret = 3; } else
        if((*s & 8) == 0 && n > 2) { arg = ((*s & 0x7)<<18)|((*(s+1) & 0x3F)<<12)|((*(s+2) & 0x3F)<<6)|(*(s+3) & 0x3F); ret = 4; }
        else return -1;
    }
    if(__pwc) *__pwc = arg;
    return ret;
}

int wctomb (char *s, wchar_t u)
{
    int ret = 0;
    if(u<0x80) {
        *s = u;
        ret = 1;
    } else if(u<0x800) {
        *(s+0)=((u>>6)&0x1F)|0xC0;
        *(s+1)=(u&0x3F)|0x80;
        ret = 2;
    } else {
        *(s+0)=((u>>12)&0x0F)|0xE0;
        *(s+1)=((u>>6)&0x3F)|0x80;
        *(s+2)=(u&0x3F)|0x80;
        ret = 3;
    }
    return ret;
}

size_t mbstowcs (wchar_t *__pwcs, const char *__s, size_t __n)
{
    int r;
    wchar_t *orig = __pwcs;
    if(!__s || !*__s) return 0;
    while(*__s) {
        r = mbtowc(__pwcs, __s, __n - (size_t)(__pwcs - orig));
        if(r < 0) return (size_t)-1;
        __pwcs++;
        __s += r;
    }
    *__pwcs = 0;
    return (size_t)(__pwcs - orig);
}

size_t wcstombs (char *__s, const wchar_t *__pwcs, size_t __n)
{
    int r;
    char *orig = __s;
    if(!__s || !__pwcs || !*__pwcs) return 0;
    while(*__pwcs && ((size_t)(__s - orig + 4) < __n)) {
        r = wctomb(__s, *__pwcs);
        if(r < 0) return (size_t)-1;
        __pwcs++;
        __s += r;
    }
    *__s = 0;
    return (size_t)(__s - orig);
}

void srand(unsigned int __seed)
{
    __srand_seed = __seed - 1;
}

int rand()
{
    efi_guid_t rngGuid = EFI_RNG_PROTOCOL_GUID;
    efi_rng_protocol_t *rng = NULL;
    efi_status_t status;
    int ret = 0;

    __srand_seed = 6364136223846793005ULL*__srand_seed + 1;
    status = BS->LocateProtocol(&rngGuid, NULL, (void**)&rng);
    if(!EFI_ERROR(status) && rng)
        rng->GetRNG(rng, NULL, (uintn_t)sizeof(int), (uint8_t*)&ret);
    ret ^= (int)(__srand_seed>>33);
    return ret;
}

uint8_t *getenv(char_t *name, uintn_t *len)
{
    efi_guid_t globGuid = EFI_GLOBAL_VARIABLE;
    uint8_t tmp[EFI_MAXIMUM_VARIABLE_SIZE], *ret;
    uint32_t attr;
    efi_status_t status;
#ifndef UEFI_NO_UTF8
    wchar_t wcname[256];
    mbstowcs((wchar_t*)&wcname, name, 256);
    status = RT->GetVariable((wchar_t*)&wcname, &globGuid, &attr, len, &tmp);
#else
    status = RT->GetVariable(name, &globGuid, &attr, len, &tmp);
#endif
    if(EFI_ERROR(status) || *len < 1 || !(ret = malloc((*len) + 1))) {
        *len = 0;
        return NULL;
    }
    memcpy(ret, tmp, *len);
    ret[*len] = 0;
    return ret;
}

int setenv(char_t *name, uintn_t len, uint8_t *data)
{
    efi_guid_t globGuid = EFI_GLOBAL_VARIABLE;
    efi_status_t status;
#ifndef UEFI_NO_UTF8
    wchar_t wcname[256];
    mbstowcs((wchar_t*)&wcname, name, 256);
    status = RT->SetVariable(wcname, &globGuid, 0, len, data);
#else
    status = RT->SetVariable(name, &globGuid, 0, len, data);
#endif
    return !EFI_ERROR(status);
}

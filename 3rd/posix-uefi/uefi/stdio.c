/*
 * stdio.c
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
 * @brief Implementing functions which are defined in stdio.h
 *
 */

#include <uefi.h>

static efi_file_handle_t *__root_dir = NULL;
static efi_serial_io_protocol_t *__ser = NULL;
static block_file_t *__blk_devs = NULL;
static uintn_t __blk_ndevs = 0;
extern time_t __mktime_efi(efi_time_t *t);
void __stdio_cleanup(void);
void __stdio_seterrno(efi_status_t status);
int __remove (const char_t *__filename, int isdir);

void __stdio_cleanup(void)
{
#ifndef UEFI_NO_UTF8
    if(__argvutf8)
        BS->FreePool(__argvutf8);
#endif
    if(__blk_devs) {
        free(__blk_devs);
        __blk_devs = NULL;
        __blk_ndevs = 0;
    }
}

void __stdio_seterrno(efi_status_t status)
{
    switch((int)(status & 0xffff)) {
        case EFI_WRITE_PROTECTED & 0xffff: errno = EROFS; break;
        case EFI_ACCESS_DENIED & 0xffff: errno = EACCES; break;
        case EFI_VOLUME_FULL & 0xffff: errno = ENOSPC; break;
        case EFI_NOT_FOUND & 0xffff: errno = ENOENT; break;
        case EFI_INVALID_PARAMETER & 0xffff: errno = EINVAL; break;
        default: errno = EIO; break;
    }
}

int fstat (FILE *__f, struct stat *__buf)
{
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t info;
    uintn_t fsiz = (uintn_t)sizeof(efi_file_info_t);
    efi_status_t status;
    uintn_t i;

    if(!__f || !__buf) {
        errno = EINVAL;
        return -1;
    }
    memset(__buf, 0, sizeof(struct stat));
    if(__f == stdin) {
        __buf->st_mode = S_IREAD | S_IFIFO;
        return 0;
    }
    if(__f == stdout || __f == stderr) {
        __buf->st_mode = S_IWRITE | S_IFIFO;
        return 0;
    }
    if(__ser && __f == (FILE*)__ser) {
        __buf->st_mode = S_IREAD | S_IWRITE | S_IFCHR;
        return 0;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(__f == (FILE*)__blk_devs[i].bio) {
            __buf->st_mode = S_IREAD | S_IWRITE | S_IFBLK;
            __buf->st_size = (off_t)__blk_devs[i].bio->Media->BlockSize * ((off_t)__blk_devs[i].bio->Media->LastBlock + 1);
            __buf->st_blocks = __blk_devs[i].bio->Media->LastBlock + 1;
            return 0;
        }
    status = __f->GetInfo(__f, &infGuid, &fsiz, &info);
    if(EFI_ERROR(status)) {
        __stdio_seterrno(status);
        return -1;
    }
    __buf->st_mode = S_IREAD |
        (info.Attribute & EFI_FILE_READ_ONLY ? 0 : S_IWRITE) |
        (info.Attribute & EFI_FILE_DIRECTORY ? S_IFDIR : S_IFREG);
    __buf->st_size = (off_t)info.FileSize;
    __buf->st_blocks = (blkcnt_t)info.PhysicalSize;
    __buf->st_atime = __mktime_efi(&info.LastAccessTime);
    __buf->st_mtime = __mktime_efi(&info.ModificationTime);
    __buf->st_ctime = __mktime_efi(&info.CreateTime);
    return 0;
}

int fclose (FILE *__stream)
{
    efi_status_t status = EFI_SUCCESS;
    uintn_t i;
    if(!__stream) {
        errno = EINVAL;
        return 0;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr || (__ser && __stream == (FILE*)__ser)) {
        return 1;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(__stream == (FILE*)__blk_devs[i].bio)
            return 1;
    status = __stream->Close(__stream);
    free(__stream);
    return !EFI_ERROR(status);
}

int fflush (FILE *__stream)
{
    efi_status_t status = EFI_SUCCESS;
    uintn_t i;
    if(!__stream) {
        errno = EINVAL;
        return 0;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr || (__ser && __stream == (FILE*)__ser)) {
        return 1;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(__stream == (FILE*)__blk_devs[i].bio) {
            return 1;
        }
    status = __stream->Flush(__stream);
    return !EFI_ERROR(status);
}

int __remove (const char_t *__filename, int isdir)
{
    efi_status_t status;
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t info;
    uintn_t fsiz = (uintn_t)sizeof(efi_file_info_t), i;
    /* little hack to support read and write mode for Delete() and stat() without create mode or checks */
    FILE *f = fopen(__filename, CL("*"));
    if(errno)
        return 1;
    if(!f || f == stdin || f == stdout || f == stderr || (__ser && f == (FILE*)__ser)) {
        errno = EBADF;
        return 1;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(f == (FILE*)__blk_devs[i].bio) {
            errno = EBADF;
            return 1;
        }
    if(isdir != -1) {
        status = f->GetInfo(f, &infGuid, &fsiz, &info);
        if(EFI_ERROR(status)) goto err;
        if(isdir == 0 && (info.Attribute & EFI_FILE_DIRECTORY)) {
            fclose(f); errno = EISDIR;
            return -1;
        }
        if(isdir == 1 && !(info.Attribute & EFI_FILE_DIRECTORY)) {
            fclose(f); errno = ENOTDIR;
            return -1;
        }
    }
    status = f->Delete(f);
    if(EFI_ERROR(status)) {
err:    __stdio_seterrno(status);
        fclose(f);
        return -1;
    }
    /* no need for fclose(f); */
    free(f);
    return 0;
}

int remove (const char_t *__filename)
{
    return __remove(__filename, -1);
}

FILE *fopen (const char_t *__filename, const char_t *__modes)
{
    FILE *ret;
    efi_status_t status;
    efi_guid_t sfsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    efi_simple_file_system_protocol_t *sfs = NULL;
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t info;
    uintn_t fsiz = (uintn_t)sizeof(efi_file_info_t), par, i;
#ifndef UEFI_NO_UTF8
    wchar_t wcname[BUFSIZ];
#endif
    errno = 0;
    if(!__filename || !*__filename || !__modes || (__modes[0] != CL('r') && __modes[0] != CL('w') && __modes[0] != CL('a') &&
      __modes[0] != CL('*')) || (__modes[1] != 0 && __modes[1] != CL('d') && __modes[1] != CL('+'))) {
        errno = EINVAL;
        return NULL;
    }
    /* fake some device names. UEFI has no concept of device files */
    if(!strcmp(__filename, CL("/dev/stdin"))) {
        if(__modes[0] == CL('w') || __modes[0] == CL('a')) { errno = EPERM; return NULL; }
        return stdin;
    }
    if(!strcmp(__filename, CL("/dev/stdout"))) {
        if(__modes[0] == CL('r')) { errno = EPERM; return NULL; }
        return stdout;
    }
    if(!strcmp(__filename, CL("/dev/stderr"))) {
        if(__modes[0] == CL('r')) { errno = EPERM; return NULL; }
        return stderr;
    }
    if(!memcmp(__filename, CL("/dev/serial"), 11 * sizeof(char_t))) {
        par = (uintn_t)atol(__filename + 11);
        if(!__ser) {
            efi_guid_t serGuid = EFI_SERIAL_IO_PROTOCOL_GUID;
            status = BS->LocateProtocol(&serGuid, NULL, (void**)&__ser);
            if(EFI_ERROR(status) || !__ser) { errno = ENOENT; return NULL; }
        }
        __ser->SetAttributes(__ser, par > 9600 ? par : 115200, 0, 1000, NoParity, 8, OneStopBit);
        return (FILE*)__ser;
    }
    if(!memcmp(__filename, CL("/dev/disk"), 9 * sizeof(char_t))) {
        par = (uintn_t)atol(__filename + 9);
        if(!__blk_ndevs) {
            efi_guid_t bioGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
            efi_handle_t handles[128];
            uintn_t handle_size = sizeof(handles);
            status = BS->LocateHandle(ByProtocol, &bioGuid, NULL, &handle_size, (efi_handle_t*)&handles);
            if(!EFI_ERROR(status)) {
                handle_size /= (uintn_t)sizeof(efi_handle_t);
                /* workaround a bug in TianoCore, it reports zero size even though the data is in the buffer */
                if(handle_size < 1)
                    handle_size = (uintn_t)sizeof(handles) / sizeof(efi_handle_t);
                __blk_devs = (block_file_t*)malloc(handle_size * sizeof(block_file_t));
                if(__blk_devs) {
                    memset(__blk_devs, 0, handle_size * sizeof(block_file_t));
                    for(i = __blk_ndevs = 0; i < handle_size; i++)
                        if(handles[i] && !EFI_ERROR(BS->HandleProtocol(handles[i], &bioGuid, (void **) &__blk_devs[__blk_ndevs].bio)) &&
                            __blk_devs[__blk_ndevs].bio && __blk_devs[__blk_ndevs].bio->Media &&
                            __blk_devs[__blk_ndevs].bio->Media->BlockSize > 0)
                                __blk_ndevs++;
                } else
                    __blk_ndevs = 0;
            }
        }
        if(__blk_ndevs && par < __blk_ndevs)
            return (FILE*)__blk_devs[par].bio;
        errno = ENOENT;
        return NULL;
    }
    if(!__root_dir && LIP) {
        status = BS->HandleProtocol(LIP->DeviceHandle, &sfsGuid, (void **)&sfs);
        if(!EFI_ERROR(status))
            status = sfs->OpenVolume(sfs, &__root_dir);
    }
    if(!__root_dir) {
        errno = ENODEV;
        return NULL;
    }
    ret = (FILE*)malloc(sizeof(FILE));
    if(!ret) return NULL;
    /* normally write means read,write,create. But for remove (internal '*' mode), we need read,write without create
     * also mode 'w' in POSIX means write-only (without read), but that's not working on certain firmware, we must
     * pass read too. This poses a problem of truncating a write-only file, see issue #26, we have to do that manually */
#ifndef UEFI_NO_UTF8
    mbstowcs((wchar_t*)&wcname, __filename, BUFSIZ - 1);
    status = __root_dir->Open(__root_dir, &ret, (wchar_t*)&wcname,
#else
    status = __root_dir->Open(__root_dir, &ret, (wchar_t*)__filename,
#endif
        __modes[0] == CL('w') || __modes[0] == CL('a') ? (EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE) :
            EFI_FILE_MODE_READ | (__modes[0] == CL('*') || __modes[1] == CL('+') ? EFI_FILE_MODE_WRITE : 0),
        __modes[1] == CL('d') ? EFI_FILE_DIRECTORY : 0);
    if(EFI_ERROR(status)) {
err:    __stdio_seterrno(status);
        free(ret); return NULL;
    }
    if(__modes[0] == CL('*')) return ret;
    status = ret->GetInfo(ret, &infGuid, &fsiz, &info);
    if(EFI_ERROR(status)) goto err;
    if(__modes[1] == CL('d') && !(info.Attribute & EFI_FILE_DIRECTORY)) {
        ret->Close(ret); free(ret); errno = ENOTDIR; return NULL;
    }
    if(__modes[1] != CL('d') && (info.Attribute & EFI_FILE_DIRECTORY)) {
        ret->Close(ret); free(ret); errno = EISDIR; return NULL;
    }
    if(__modes[0] == CL('a')) fseek(ret, 0, SEEK_END);
    if(__modes[0] == CL('w')) {
        /* manually truncate file size
         * See https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.c
         * function FileHandleSetSize */
        info.FileSize = 0;
        ret->SetInfo(ret, &infGuid, fsiz, &info);
    }
    return ret;
}

size_t fread (void *__ptr, size_t __size, size_t __n, FILE *__stream)
{
    uintn_t bs = __size * __n, i, n;
    efi_status_t status;
    if(!__ptr || __size < 1 || __n < 1 || !__stream) {
        errno = EINVAL;
        return 0;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr) {
        errno = ESPIPE;
        return 0;
    }
    if(__ser && __stream == (FILE*)__ser) {
        status = __ser->Read(__ser, &bs, __ptr);
    } else {
        for(i = 0; i < __blk_ndevs; i++)
            if(__stream == (FILE*)__blk_devs[i].bio) {
                n = __blk_devs[i].offset / __blk_devs[i].bio->Media->BlockSize;
                bs = (bs / __blk_devs[i].bio->Media->BlockSize) * __blk_devs[i].bio->Media->BlockSize;
                status = __blk_devs[i].bio->ReadBlocks(__blk_devs[i].bio, __blk_devs[i].bio->Media->MediaId, n, bs, __ptr);
                if(EFI_ERROR(status)) {
                    __stdio_seterrno(status);
                    return 0;
                }
                __blk_devs[i].offset += bs;
                return bs / __size;
            }
        status = __stream->Read(__stream, &bs, __ptr);
    }
    if(EFI_ERROR(status)) {
        __stdio_seterrno(status);
        return 0;
    }
    return bs / __size;
}

size_t fwrite (const void *__ptr, size_t __size, size_t __n, FILE *__stream)
{
    uintn_t bs = __size * __n, n, i;
    efi_status_t status;
    if(!__ptr || __size < 1 || __n < 1 || !__stream) {
        errno = EINVAL;
        return 0;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr) {
        errno = ESPIPE;
        return 0;
    }
    if(__ser && __stream == (FILE*)__ser) {
        status = __ser->Write(__ser, &bs, (void*)__ptr);
    } else {
        for(i = 0; i < __blk_ndevs; i++)
            if(__stream == (FILE*)__blk_devs[i].bio) {
                n = __blk_devs[i].offset / __blk_devs[i].bio->Media->BlockSize;
                bs = (bs / __blk_devs[i].bio->Media->BlockSize) * __blk_devs[i].bio->Media->BlockSize;
                status = __blk_devs[i].bio->WriteBlocks(__blk_devs[i].bio, __blk_devs[i].bio->Media->MediaId, n, bs, (void*)__ptr);
                if(EFI_ERROR(status)) {
                    __stdio_seterrno(status);
                    return 0;
                }
                __blk_devs[i].offset += bs;
                return bs / __size;
            }
        status = __stream->Write(__stream, &bs, (void *)__ptr);
    }
    if(EFI_ERROR(status)) {
        __stdio_seterrno(status);
        return 0;
    }
    return bs / __size;
}

int fseek (FILE *__stream, long int __off, int __whence)
{
    off_t off = 0;
    efi_status_t status;
    efi_guid_t infoGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t info;
    uintn_t fsiz = sizeof(efi_file_info_t), i;
    if(!__stream || (__whence != SEEK_SET && __whence != SEEK_CUR && __whence != SEEK_END)) {
        errno = EINVAL;
        return -1;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr) {
        errno = ESPIPE;
        return -1;
    }
    if(__ser && __stream == (FILE*)__ser) {
        errno = EBADF;
        return -1;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(__stream == (FILE*)__blk_devs[i].bio) {
            off = (uint64_t)__blk_devs[i].bio->Media->BlockSize * (uint64_t)__blk_devs[i].bio->Media->LastBlock;
            switch(__whence) {
                case SEEK_END:
                    __blk_devs[i].offset = off + __off;
                    break;
                case SEEK_CUR:
                    __blk_devs[i].offset += __off;
                    break;
                case SEEK_SET:
                    __blk_devs[i].offset = __off;
                    break;
            }
            if(__blk_devs[i].offset < 0) __blk_devs[i].offset = 0;
            if(__blk_devs[i].offset > off) __blk_devs[i].offset = off;
            __blk_devs[i].offset = (__blk_devs[i].offset / __blk_devs[i].bio->Media->BlockSize) *
                __blk_devs[i].bio->Media->BlockSize;
            return 0;
        }
    switch(__whence) {
        case SEEK_END:
            status = __stream->GetInfo(__stream, &infoGuid, &fsiz, &info);
            if(!EFI_ERROR(status)) {
                off = info.FileSize + __off;
                status = __stream->SetPosition(__stream, off);
            }
            break;
        case SEEK_CUR:
            status = __stream->GetPosition(__stream, &off);
            if(!EFI_ERROR(status)) {
                off += __off;
                status = __stream->SetPosition(__stream, off);
            }
            break;
        default:
            status = __stream->SetPosition(__stream, __off);
            break;
    }
    return EFI_ERROR(status) ? -1 : 0;
}

long int ftell (FILE *__stream)
{
    uint64_t off = 0;
    uintn_t i;
    efi_status_t status;
    if(!__stream) {
        errno = EINVAL;
        return -1;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr) {
        errno = ESPIPE;
        return -1;
    }
    if(__ser && __stream == (FILE*)__ser) {
        errno = EBADF;
        return -1;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(__stream == (FILE*)__blk_devs[i].bio) {
            return (long int)__blk_devs[i].offset;
        }
    status = __stream->GetPosition(__stream, &off);
    return EFI_ERROR(status) ? -1 : (long int)off;
}

int feof (FILE *__stream)
{
    uint64_t off = 0;
    efi_guid_t infGuid = EFI_FILE_INFO_GUID;
    efi_file_info_t info;
    uintn_t fsiz = (uintn_t)sizeof(efi_file_info_t), i;
    efi_status_t status;
    if(!__stream) {
        errno = EINVAL;
        return 0;
    }
    if(__stream == stdin || __stream == stdout || __stream == stderr) {
        errno = ESPIPE;
        return 0;
    }
    if(__ser && __stream == (FILE*)__ser) {
        errno = EBADF;
        return 0;
    }
    for(i = 0; i < __blk_ndevs; i++)
        if(__stream == (FILE*)__blk_devs[i].bio) {
            errno = EBADF;
            return __blk_devs[i].offset == (off_t)__blk_devs[i].bio->Media->BlockSize * (off_t)__blk_devs[i].bio->Media->LastBlock;
        }
    status = __stream->GetPosition(__stream, &off);
    if(EFI_ERROR(status)) {
err:    __stdio_seterrno(status);
        return 1;
    }
    status = __stream->GetInfo(__stream, &infGuid, &fsiz, &info);
    if(EFI_ERROR(status)) goto err;
    __stream->SetPosition(__stream, off);
    return info.FileSize == off;
}

int vsnprintf(char_t *dst, size_t maxlen, const char_t *fmt, __builtin_va_list args)
{
#define needsescape(a) (a==CL('\"') || a==CL('\\') || a==CL('\a') || a==CL('\b') || a==CL('\033') || a==CL('\f') || \
    a==CL('\r') || a==CL('\n') || a==CL('\t') || a==CL('\v'))
    efi_physical_address_t m;
    uint8_t *mem;
    int64_t arg;
    int len, sign, i, j;
    char_t *p, *orig=dst, *end = dst + maxlen - 1, tmpstr[24], pad, n;
#ifdef UEFI_NO_UTF8
    char *c;
#endif
    if(dst==NULL || fmt==NULL)
        return 0;

    arg = 0;
    while(*fmt && dst < end) {
        if(*fmt==CL('%')) {
            fmt++;
            if(*fmt==CL('%')) goto put;
            len=0; pad=CL(' ');
            if(*fmt==CL('0')) pad=CL('0');
            while(*fmt>=CL('0') && *fmt<=CL('9')) {
                len *= 10;
                len += *fmt-CL('0');
                fmt++;
            }
            if(*fmt==CL('l')) fmt++;
            if(*fmt==CL('c')) {
                arg = __builtin_va_arg(args, uint32_t);
#ifndef UEFI_NO_UTF8
                if(arg<0x80) { *dst++ = arg; } else
                if(arg<0x800) { *dst++ = ((arg>>6)&0x1F)|0xC0; *dst++ = (arg&0x3F)|0x80; } else
                { *dst++ = ((arg>>12)&0x0F)|0xE0; *dst++ = ((arg>>6)&0x3F)|0x80; *dst++ = (arg&0x3F)|0x80; }
#else
                *dst++ = (wchar_t)(arg & 0xffff);
#endif
                fmt++;
                continue;
            } else
            if(*fmt==CL('d') || *fmt==CL('i')) {
                arg = __builtin_va_arg(args, int64_t);
                sign=0;
                if(arg<0) {
                    arg*=-1;
                    sign++;
                }
                i=23;
                tmpstr[i]=0;
                do {
                    tmpstr[--i]=CL('0')+(arg%10);
                    arg/=10;
                } while(arg!=0 && i>0);
                if(sign) {
                    tmpstr[--i]=CL('-');
                }
                if(len>0 && len<23) {
                    while(i && i>23-len) {
                        tmpstr[--i]=pad;
                    }
                }
                p=&tmpstr[i];
                goto copystring;
            } else
            if(*fmt==CL('p')) {
                arg = __builtin_va_arg(args, uint64_t);
                len = 16; pad = CL('0'); goto hex;
            } else
            if(*fmt==CL('x') || *fmt==CL('X')) {
                arg = __builtin_va_arg(args, int64_t);
hex:            i=16;
                tmpstr[i]=0;
                do {
                    n=arg & 0xf;
                    /* 0-9 => '0'-'9', 10-15 => 'A'-'F' */
                    tmpstr[--i]=n+(n>9?(*fmt==CL('X')?0x37:0x57):0x30);
                    arg>>=4;
                } while(arg!=0 && i>0);
                /* padding, only leading zeros */
                if(len>0 && len<=16) {
                    while(i>16-len) {
                        tmpstr[--i]=CL('0');
                    }
                }
                p=&tmpstr[i];
                goto copystring;
            } else
            if(*fmt==CL('s') || *fmt==CL('q')) {
                p = __builtin_va_arg(args, char_t*);
copystring:     if(p==NULL) {
                    p=CL("(null)");
                }
                while(*p && dst + 2 < end) {
                    if(*fmt==CL('q') && needsescape(*p)) {
                        *dst++ = CL('\\');
                        switch(*p) {
                            case CL('\a'): *dst++ = CL('a'); break;
                            case CL('\b'): *dst++ = CL('b'); break;
                            case 27:       *dst++ = CL('e'); break; /* gcc 10.2 doesn't like CL('\e') in ansi mode */
                            case CL('\f'): *dst++ = CL('f'); break;
                            case CL('\n'): *dst++ = CL('n'); break;
                            case CL('\r'): *dst++ = CL('r'); break;
                            case CL('\t'): *dst++ = CL('t'); break;
                            case CL('\v'): *dst++ = CL('v'); break;
                            default: *dst++ = *p++; break;
                        }
                    } else {
                        if(*p == CL('\n') && (orig == dst || *(dst - 1) != CL('\r'))) *dst++ = CL('\r');
                        *dst++ = *p++;
                    }
                }
            } else
#ifdef UEFI_NO_UTF8
            if(*fmt==L'S' || *fmt==L'Q') {
                c = __builtin_va_arg(args, char*);
                if(c==NULL) goto copystring;
                while(*p && dst + 2 < end) {
                    arg = *c;
                    if((*c & 128) != 0) {
                        if((*c & 32) == 0 ) {
                            arg = ((*c & 0x1F)<<6)|(*(c+1) & 0x3F);
                            c += 1;
                        } else
                        if((*c & 16) == 0 ) {
                            arg = ((*c & 0xF)<<12)|((*(c+1) & 0x3F)<<6)|(*(c+2) & 0x3F);
                            c += 2;
                        } else
                        if((*c & 8) == 0 ) {
                            arg = ((*c & 0x7)<<18)|((*(c+1) & 0x3F)<<12)|((*(c+2) & 0x3F)<<6)|(*(c+3) & 0x3F);
                            c += 3;
                        } else
                            arg = L'?';
                    }
                    if(!arg) break;
                    if(*fmt==L'Q' && needsescape(arg)) {
                        *dst++ = L'\\';
                        switch(arg) {
                            case L'\a': *dst++ = L'a'; break;
                            case L'\b': *dst++ = L'b'; break;
                            case 27:    *dst++ = L'e'; break;   /* gcc 10.2 doesn't like L'\e' in ansi mode */
                            case L'\f': *dst++ = L'f'; break;
                            case L'\n': *dst++ = L'n'; break;
                            case L'\r': *dst++ = L'r'; break;
                            case L'\t': *dst++ = L't'; break;
                            case L'\v': *dst++ = L'v'; break;
                            default: *dst++ = arg; break;
                        }
                    } else {
                        if(arg == L'\n') *dst++ = L'\r';
                        *dst++ = (wchar_t)(arg & 0xffff);
                    }
                }
            } else
#endif
            if(*fmt==CL('D')) {
                m = __builtin_va_arg(args, efi_physical_address_t);
                for(j = 0; j < (len < 1 ? 1 : (len > 16 ? 16 : len)); j++) {
                    for(i = 44; i >= 0; i -= 4) {
                        n = (m >> i) & 15; *dst++ = n + (n>9?0x37:0x30);
                        if(dst >= end) goto zro;
                    }
                    *dst++ = CL(':'); if(dst >= end) goto zro;
                    *dst++ = CL(' '); if(dst >= end) goto zro;
                    mem = (uint8_t*)m;
                    for(i = 0; i < 16; i++) {
                        n = (mem[i] >> 4) & 15; *dst++ = n + (n>9?0x37:0x30); if(dst >= end) goto zro;
                        n = mem[i] & 15; *dst++ = n + (n>9?0x37:0x30); if(dst >= end) goto zro;
                        *dst++ = CL(' ');if(dst >= end) goto zro;
                    }
                    *dst++ = CL(' '); if(dst >= end) goto zro;
                    for(i = 0; i < 16; i++) {
                        *dst++ = (mem[i] < 32 || mem[i] >= 127 ? CL('.') : (char_t)mem[i]);
                        if(dst >= end) goto zro;
                    }
                    *dst++ = CL('\r'); if(dst >= end) goto zro;
                    *dst++ = CL('\n'); if(dst >= end) goto zro;
                    m += 16;
                }
            }
        } else {
put:        if(*fmt == CL('\n') && (orig == dst || *(dst - 1) != CL('\r'))) *dst++ = CL('\r');
            *dst++ = *fmt;
        }
        fmt++;
    }
zro:*dst=0;
    return (int)(dst-orig);
#undef needsescape
}

int vsprintf(char_t *dst, const char_t *fmt, __builtin_va_list args)
{
    return vsnprintf(dst, BUFSIZ, fmt, args);
}

int sprintf(char_t *dst, const char_t* fmt, ...)
{
    int ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    ret = vsnprintf(dst, BUFSIZ, fmt, args);
    __builtin_va_end(args);
    return ret;
}

int snprintf(char_t *dst, size_t maxlen, const char_t* fmt, ...)
{
    int ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    ret = vsnprintf(dst, maxlen, fmt, args);
    __builtin_va_end(args);
    return ret;
}

int vprintf(const char_t* fmt, __builtin_va_list args)
{
    int ret;
    wchar_t dst[BUFSIZ];
#ifndef UEFI_NO_UTF8
    char_t tmp[BUFSIZ];
    ret = vsnprintf(tmp, BUFSIZ, fmt, args);
    mbstowcs(dst, tmp, BUFSIZ - 1);
#else
    ret = vsnprintf(dst, BUFSIZ, fmt, args);
#endif
    ST->ConOut->OutputString(ST->ConOut, (wchar_t *)&dst);
    return ret;
}

int printf(const char_t* fmt, ...)
{
    int ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    ret = vprintf(fmt, args);
    __builtin_va_end(args);
    return ret;
}

int vfprintf (FILE *__stream, const char_t *__format, __builtin_va_list args)
{
    wchar_t dst[BUFSIZ];
    char_t tmp[BUFSIZ];
    uintn_t ret, i;
#ifndef UEFI_NO_UTF8
    ret = (uintn_t)vsnprintf(tmp, BUFSIZ, __format, args);
    ret = mbstowcs(dst, tmp, BUFSIZ - 1);
#else
    ret = vsnprintf(dst, BUFSIZ, __format, args);
#endif
    if(ret < 1 || !__stream || __stream == stdin) return 0;
    for(i = 0; i < __blk_ndevs; i++)
        if(__stream == (FILE*)__blk_devs[i].bio) {
            errno = EBADF;
            return -1;
        }
    if(__stream == stdout)
        ST->ConOut->OutputString(ST->ConOut, (wchar_t*)&dst);
    else if(__stream == stderr)
        ST->StdErr->OutputString(ST->StdErr, (wchar_t*)&dst);
    else if(__ser && __stream == (FILE*)__ser) {
#ifdef UEFI_NO_UTF8
        wcstombs((char*)&tmp, dst, BUFSIZ - 1);
#endif
        __ser->Write(__ser, &ret, (void*)&tmp);
    } else
#ifndef UEFI_NO_UTF8
        __stream->Write(__stream, &ret, (void*)&tmp);
#else
        __stream->Write(__stream, &ret, (void*)&dst);
#endif
    return (int)ret;
}

int fprintf (FILE *__stream, const char_t *__format, ...)
{
    int ret;
    __builtin_va_list args;
    __builtin_va_start(args, __format);
    ret = vfprintf(__stream, __format, args);
    __builtin_va_end(args);
    return ret;
}

int getchar_ifany (void)
{
    efi_input_key_t key = { 0 };
    efi_status_t status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
    return EFI_ERROR(status) ? 0 : key.UnicodeChar;
}

int getchar (void)
{
    uintn_t idx;
    BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &idx);
    return getchar_ifany();
}

int putchar (int __c)
{
    wchar_t tmp[2];
    tmp[0] = (wchar_t)__c;
    tmp[1] = 0;
    ST->ConOut->OutputString(ST->ConOut, (__c == L'\n' ? (wchar_t*)L"\r\n" : (wchar_t*)&tmp));
    return (int)tmp[0];
}


/**
 * @file ff.h
 * @brief fat 文件系统头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_FF_H
#define SIMPLEKERNEL_FF_H

/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem module  R0.15                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2022, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:

/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

#ifndef FF_DEFINED
/* Revision ID */
#    define FF_DEFINED 80286

#    ifdef __cplusplus
extern "C" {
#    endif

/* FatFs configuration options */
#    include "ffconf.h"

#    if FF_DEFINED != FFCONF_DEF
#        error Wrong configuration file (ffconf.h).
#    endif

/* Integer types used for FatFs API */

/* Windows VC++ (for development only) */
#    if defined(_WIN32)
#        define FF_INTDEF 2
#        include <windows.h>
typedef unsigned __int64 QWORD;
#        include <float.h>
#        define isnan(_v) _isnan(_v)
#        define isinf(_v) (!_finite(_v))

#    elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
      || defined(__cplusplus) /* C99 or later */
#        define FF_INTDEF 2
#        include <stdint.h>
/* int must be 16-bit or 32-bit */
typedef unsigned int  UINT;
/* char must be 8-bit */
typedef unsigned char BYTE;
/* 16-bit unsigned integer */
typedef uint16_t      WORD;
/* 32-bit unsigned integer */
typedef uint32_t      DWORD;
/* 64-bit unsigned integer */
typedef uint64_t      QWORD;
/* UTF-16 character type */
typedef WORD          WCHAR;

/* Earlier than C99 */
#    else
#        define FF_INTDEF 1
/* int must be 16-bit or 32-bit */
typedef unsigned int   UINT;
/* char must be 8-bit */
typedef unsigned char  BYTE;
/* 16-bit unsigned integer */
typedef unsigned short WORD;
/* 32-bit unsigned integer */
typedef unsigned long  DWORD;
/* UTF-16 character type */
typedef WORD           WCHAR;
#    endif

/* Type of file size and LBA variables */

#    if FF_FS_EXFAT
#        if FF_INTDEF != 2
#            error exFAT feature wants C99 or later
#        endif
typedef QWORD FSIZE_t;
#        if FF_LBA64
typedef QWORD LBA_t;
#        else
typedef DWORD LBA_t;
#        endif
#    else
#        if FF_LBA64
#            error exFAT needs to be enabled when enable 64-bit LBA
#        endif
typedef DWORD         FSIZE_t;
typedef DWORD         LBA_t;
#    endif

/* Type of path name strings on FatFs API (TCHAR) */

/* Unicode in UTF-16 encoding */
#    if FF_USE_LFN && FF_LFN_UNICODE == 1
typedef WCHAR TCHAR;
#        define _T(_x)    L##_x
#        define _TEXT(_x) L##_x
/* Unicode in UTF-8 encoding */
#    elif FF_USE_LFN && FF_LFN_UNICODE == 2
typedef char          TCHAR;
#        define _T(_x)    u8##_x
#        define _TEXT(_x) u8##_x
/* Unicode in UTF-32 encoding */
#    elif FF_USE_LFN && FF_LFN_UNICODE == 3
typedef DWORD          TCHAR;
#        define _T(_x)    U##_x
#        define _TEXT(_x) U##_x
#    elif FF_USE_LFN && (FF_LFN_UNICODE < 0 || FF_LFN_UNICODE > 3)
#        error Wrong FF_LFN_UNICODE setting
/* ANSI/OEM code in SBCS/DBCS */
#    else
typedef char TCHAR;
#        define _T(_x)    _x
#        define _TEXT(_x) _x
#    endif

/* Definitions of volume management */

/* Multiple partition configuration */
#    if FF_MULTI_PARTITION
typedef struct {
    /* Physical drive number */
    BYTE pd;
    /* Partition: 0:Auto detect, 1-4:Forced partition) */
    BYTE pt;
} PARTITION;

/* Volume - Partition mapping table */
extern PARTITION VolToPart[];
#    endif

#    if FF_STR_VOLUME_ID
#        ifndef FF_VOLUME_STRS
/* User defied volume ID */
extern const char* VolumeStr[FF_VOLUMES];
#        endif
#    endif

/* Filesystem object structure (FATFS) */
typedef struct {
    /* Filesystem type (0:not mounted) */
    BYTE fs_type;
    /* Volume hosting physical drive */
    BYTE pdrv;
    /* Logical drive number (used only when FF_FS_REENTRANT) */
    BYTE ldrv;
    /* Number of FATs (1 or 2) */
    BYTE n_fats;
    /* win[] status (b0:dirty) */
    BYTE wflag;
    /* FSINFO status (b7:disabled, b0:dirty) */
    BYTE fsi_flag;
    /* Volume mount ID */
    WORD id;
    /* Number of root directory entries (FAT12/16) */
    WORD n_rootdir;
    /* Cluster size [sectors] */
    WORD csize;
#    if FF_MAX_SS != FF_MIN_SS
    /* Sector size (512, 1024, 2048 or 4096) */
    WORD ssize;
#    endif
#    if FF_USE_LFN
    /* LFN working buffer */
    WCHAR* lfnbuf;
#    endif
#    if FF_FS_EXFAT
    /* Directory entry block scratchpad buffer for exFAT */
    BYTE* dirbuf;
#    endif
#    if !FF_FS_READONLY
    /* Last allocated cluster */
    DWORD last_clst;
    /* Number of free clusters */
    DWORD free_clst;
#    endif
#    if FF_FS_RPATH
    /* Current directory start cluster (0:root) */
    DWORD cdir;
#        if FF_FS_EXFAT
    /* Containing directory start cluster (invalid when cdir is 0) */
    DWORD cdc_scl;
    /* b31-b8:Size of containing directory, b7-b0: Chain status */
    DWORD cdc_size;
    /* Offset in the containing directory (invalid when cdir is 0) */
    DWORD cdc_ofs;
#        endif
#    endif
    /* Number of FAT entries (number of clusters + 2) */
    DWORD n_fatent;
    /* Number of sectors per FAT */
    DWORD fsize;
    /* Volume base sector */
    LBA_t volbase;
    /* FAT base sector */
    LBA_t fatbase;
    /* Root directory base sector (FAT12/16) or cluster (FAT32/exFAT) */
    LBA_t dirbase;
    /* Data base sector */
    LBA_t database;
#    if FF_FS_EXFAT
    /* Allocation bitmap base sector */
    LBA_t bitbase;
#    endif
    /* Current sector appearing in the win[] */
    LBA_t winsect;
    /* Disk access window for Directory, FAT (and file data at tiny cfg) */
    BYTE  win[FF_MAX_SS];
} FATFS;

/* Object ID and allocation information (FFOBJID) */
typedef struct {
    /* Pointer to the hosting volume of this object */
    FATFS*  fs;
    /* Hosting volume's mount ID */
    WORD    id;
    /* Object attribute */
    BYTE    attr;
    /* Object chain status (b1-0: =0:not contiguous, =2:contiguous,
     * =3:fragmented in this session, b2:sub-directory stretched) */
    BYTE    stat;
    /* Object data start cluster (0:no cluster or root directory) */
    DWORD   sclust;
    /* Object size (valid when sclust != 0) */
    FSIZE_t objsize;
#    if FF_FS_EXFAT
    /* Size of first fragment - 1 (valid when stat == 3) */
    DWORD n_cont;
    /* Size of last fragment needs to be written to FAT (valid when not zero) */
    DWORD n_frag;
    /* Containing directory start cluster (valid when sclust != 0) */
    DWORD c_scl;
    /* b31-b8:Size of containing directory, b7-b0: Chain status (valid when
     * c_scl != 0) */
    DWORD c_size;
    /* Offset in the containing directory (valid when file object and sclust !=
     * 0) */
    DWORD c_ofs;
#    endif
#    if FF_FS_LOCK
    /* File lock ID origin from 1 (index of file semaphore table Files[]) */
    UINT lockid;
#    endif
} FFOBJID;

/* File object structure (FIL) */
typedef struct {
    /* Object identifier (must be the 1st member to detect invalid object
     * pointer) */
    FFOBJID obj;
    /* File status flags */
    BYTE    flag;
    /* Abort flag (error code) */
    BYTE    err;
    /* File read/write pointer (Zeroed on file open) */
    FSIZE_t fptr;
    /* Current cluster of fpter (invalid when fptr is 0) */
    DWORD   clust;
    /* Sector number appearing in buf[] (0:invalid) */
    LBA_t   sect;
#    if !FF_FS_READONLY
    /* Sector number containing the directory entry (not used at exFAT) */
    LBA_t dir_sect;
    /* Pointer to the directory entry in the win[] (not used at exFAT) */
    BYTE* dir_ptr;
#    endif
#    if FF_USE_FASTSEEK
    /* Pointer to the cluster link map table (nulled on open, set by
     * application) */
    DWORD* cltbl;
#    endif
#    if !FF_FS_TINY
    /* File private data read/write window */
    BYTE buf[FF_MAX_SS];
#    endif
} FIL;

/* Directory object structure (DIR) */
typedef struct {
    /* Object identifier */
    FFOBJID obj;
    /* Current read/write offset */
    DWORD   dptr;
    /* Current cluster */
    DWORD   clust;
    /* Current sector (0:Read operation has terminated) */
    LBA_t   sect;
    /* Pointer to the directory item in the win[] */
    BYTE*   dir;
    /* SFN (in/out) {body[8],ext[3],status[1]} */
    BYTE    fn[12];
#    if FF_USE_LFN
    /* Offset of current entry block being processed (0xFFFFFFFF:Invalid) */
    DWORD blk_ofs;
#    endif
#    if FF_USE_FIND
    /* Pointer to the name matching pattern */
    const TCHAR* pat;
#    endif
} DIR;

/* File information structure (FILINFO) */
typedef struct {
    /* File size */
    FSIZE_t fsize;
    /* Modified date */
    WORD    fdate;
    /* Modified time */
    WORD    ftime;
    /* File attribute */
    BYTE    fattrib;
#    if FF_USE_LFN
    /* Alternative file name */
    TCHAR altname[FF_SFN_BUF + 1];
    /* Primary file name */
    TCHAR fname[FF_LFN_BUF + 1];
#    else
    /* File name */
    TCHAR fname[12 + 1];
#    endif
} FILINFO;

/* Format parameter structure (MKFS_PARM) */
typedef struct {
    /* Format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD) */
    BYTE  fmt;
    /* Number of FATs */
    BYTE  n_fat;
    /* Data area alignment (sector) */
    UINT  align;
    /* Number of root directory entries */
    UINT  n_root;
    /* Cluster size (byte) */
    DWORD au_size;
} MKFS_PARM;

/* File function return code (FRESULT) */
typedef enum {
    /* (0) Succeeded */
    FR_OK = 0,
    /* (1) A hard error occurred in the low level disk I/O layer */
    FR_DISK_ERR,
    /* (2) Assertion failed */
    FR_INT_ERR,
    /* (3) The physical drive cannot work */
    FR_NOT_READY,
    /* (4) Could not find the file */
    FR_NO_FILE,
    /* (5) Could not find the path */
    FR_NO_PATH,
    /* (6) The path name format is invalid */
    FR_INVALID_NAME,
    /* (7) Access denied due to prohibited access or directory full */
    FR_DENIED,
    /* (8) Access denied due to prohibited access */
    FR_EXIST,
    /* (9) The file/directory object is invalid */
    FR_INVALID_OBJECT,
    /* (10) The physical drive is write protected */
    FR_WRITE_PROTECTED,
    /* (11) The logical drive number is invalid */
    FR_INVALID_DRIVE,
    /* (12) The volume has no work area */
    FR_NOT_ENABLED,
    /* (13) There is no valid FAT volume */
    FR_NO_FILESYSTEM,
    /* (14) The f_mkfs() aborted due to any problem */
    FR_MKFS_ABORTED,
    /* (15) Could not get a grant to access the volume within defined period */
    FR_TIMEOUT,
    /* (16) The operation is rejected according to the file sharing policy */
    FR_LOCKED,
    /* (17) LFN working buffer could not be allocated */
    FR_NOT_ENOUGH_CORE,
    /* (18) Number of open files > FF_FS_LOCK */
    FR_TOO_MANY_OPEN_FILES,
    /* (19) Given parameter is invalid */
    FR_INVALID_PARAMETER
} FRESULT;

/*--------------------------------------------------------------*/
/* FatFs Module Application Interface                           */
/*--------------------------------------------------------------*/

/* Open or create a file */
FRESULT f_open(FIL* _fp, const TCHAR* _path, BYTE _mode);
/* Close an open file object */
FRESULT f_close(FIL* _fp);
/* Read data from the file */
FRESULT f_read(FIL* _fp, void* _buff, UINT _btr, UINT* _br);
/* Write data to the file */
FRESULT f_write(FIL* _fp, const void* _buff, UINT _btw, UINT* _bw);
/* Move file pointer of the file object */
FRESULT f_lseek(FIL* _fp, FSIZE_t _ofs);
/* Truncate the file */
FRESULT f_truncate(FIL* _fp);
/* Flush cached data of the writing file */
FRESULT f_sync(FIL* _fp);
/* Open a directory */
FRESULT f_opendir(DIR* _dp, const TCHAR* _path);
/* Close an open directory */
FRESULT f_closedir(DIR* _dp);
/* Read a directory item */
FRESULT f_readdir(DIR* _dp, FILINFO* _fno);
/* Find first file */
FRESULT
f_findfirst(DIR* _dp, FILINFO* _fno, const TCHAR* _path, const TCHAR* _pattern);
/* Find next file */
FRESULT f_findnext(DIR* _dp, FILINFO* _fno);
/* Create a sub directory */
FRESULT f_mkdir(const TCHAR* _path);
/* Delete an existing file or directory */
FRESULT f_unlink(const TCHAR* _path);
/* Rename/Move a file or directory */
FRESULT f_rename(const TCHAR* _path_old, const TCHAR* _path_new);
/* Get file status */
FRESULT f_stat(const TCHAR* _path, FILINFO* _fno);
/* Change attribute of a file/dir */
FRESULT f_chmod(const TCHAR* _path, BYTE _attr, BYTE _mask);
/* Change timestamp of a file/dir */
FRESULT f_utime(const TCHAR* _path, const FILINFO* _fno);
/* Change current directory */
FRESULT f_chdir(const TCHAR* _path);
/* Change current drive */
FRESULT f_chdrive(const TCHAR* _path);
/* Get current directory */
FRESULT f_getcwd(TCHAR* _buff, UINT _len);
/* Get number of free clusters on the drive */
FRESULT f_getfree(const TCHAR* _path, DWORD* _nclst, FATFS** _fatfs);
/* Get volume label */
FRESULT f_getlabel(const TCHAR* _path, TCHAR* _label, DWORD* _vsn);
/* Set volume label */
FRESULT f_setlabel(const TCHAR* _label);
/* Forward data to the stream */
FRESULT
f_forward(FIL* _fp, UINT (*_func)(const BYTE*, UINT), UINT _btf, UINT* _bf);
/* Allocate a contiguous block to the file */
FRESULT f_expand(FIL* _fp, FSIZE_t _fsz, BYTE _opt);
/* Mount/Unmount a logical drive */
FRESULT f_mount(FATFS* _fs, const TCHAR* _path, BYTE _opt);
/* Create a FAT volume */
FRESULT
f_mkfs(const TCHAR* _path, const MKFS_PARM* _opt, void* _work, UINT _len);
/* Divide a physical drive into some partitions */
FRESULT f_fdisk(BYTE _pdrv, const LBA_t _ptbl[], void* _work);
/* Set current code page */
FRESULT f_setcp(WORD _cp);
/* Put a character to the file */
int     f_putc(TCHAR _c, FIL* _fp);
/* Put a string to the file */
int     f_puts(const TCHAR* _str, FIL* _cp);
/* Put a formatted string to the file */
int     f_printf(FIL* _fp, const TCHAR* _str, ...);
/* Get a string from the file */
TCHAR*  f_gets(TCHAR* _buff, int _len, FIL* _fp);

/* Some API fucntions are implemented as macro */

#    define f_eof(_fp)       ((int)((_fp)->fptr == (_fp)->obj.objsize))
#    define f_error(_fp)     ((_fp)->err)
#    define f_tell(_fp)      ((_fp)->fptr)
#    define f_size(_fp)      ((_fp)->obj.objsize)
#    define f_rewind(_fp)    f_lseek((_fp), 0)
#    define f_rewinddir(_dp) f_readdir((_dp), 0)
#    define f_rmdir(_path)   f_unlink(_path)
#    define f_unmount(_path) f_mount(0, _path, 0)

/*--------------------------------------------------------------*/
/* Additional Functions                                         */
/*--------------------------------------------------------------*/

/* RTC function (provided by user) */
#    if !FF_FS_READONLY && !FF_FS_NORTC
/* Get current time */
DWORD get_fattime(void);
#    endif

/* LFN support functions (defined in ffunicode.c) */

#    if FF_USE_LFN >= 1
/* OEM code to Unicode conversion */
WCHAR ff_oem2uni(WCHAR _oem, WORD _cp);
/* Unicode to OEM code conversion */
WCHAR ff_uni2oem(DWORD _uni, WORD _cp);
/* Unicode upper-case conversion */
DWORD ff_wtoupper(DWORD _uni);
#    endif

/* O/S dependent functions (samples available in ffsystem.c) */

/* Dynamic memory allocation */
#    if FF_USE_LFN == 3
/* Allocate memory block */
void* ff_memalloc(UINT _msize);
/* Free memory block */
void  ff_memfree(void* _mblock);
#    endif
/* Sync functions */
#    if FF_FS_REENTRANT
/* Create a sync object */
int  ff_mutex_create(int _vol);
/* Delete a sync object */
void ff_mutex_delete(int _vol);
/* Lock sync object */
int  ff_mutex_take(int _vol);
/* Unlock sync object */
void ff_mutex_give(int _vol);
#    endif

/*--------------------------------------------------------------*/
/* Flags and Offset Address                                     */
/*--------------------------------------------------------------*/

/* File access mode and open method flags (3rd argument of f_open) */
#    define FA_READ          0x01
#    define FA_WRITE         0x02
#    define FA_OPEN_EXISTING 0x00
#    define FA_CREATE_NEW    0x04
#    define FA_CREATE_ALWAYS 0x08
#    define FA_OPEN_ALWAYS   0x10
#    define FA_OPEN_APPEND   0x30

/* Fast seek controls (2nd argument of f_lseek) */
#    define CREATE_LINKMAP   ((FSIZE_t)0 - 1)

/* Format options (2nd argument of f_mkfs) */
#    define FM_FAT           0x01
#    define FM_FAT32         0x02
#    define FM_EXFAT         0x04
#    define FM_ANY           0x07
#    define FM_SFD           0x08

/* Filesystem type (FATFS.fs_type) */
#    define FS_FAT12         1
#    define FS_FAT16         2
#    define FS_FAT32         3
#    define FS_EXFAT         4

/* File attribute bits for directory entry (FILINFO.fattrib) */
/* Read only */
#    define AM_RDO           0x01
/* Hidden */
#    define AM_HID           0x02
/* System */
#    define AM_SYS           0x04
/* Directory */
#    define AM_DIR           0x10
/* Archive */
#    define AM_ARC           0x20

#    ifdef __cplusplus
}
#    endif

#endif /* FF_DEFINED */

#endif /* SIMPLEKERNEL_FF_H */

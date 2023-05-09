
/**
 * @file ff.c
 * @brief FatFs - Generic FAT Filesystem Module  R0.15 w/patch1
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on http://elm-chan.org/fsw/ff/00index_e.html
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

/*----------------------------------------------------------------------------/
/
/ Copyright (C) 2022, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

#include <string.h>
/// Declarations of FatFs API
#include "ff.h"
/// Declarations of device I/O functions
#include "diskio.h"

// Module Private Definitions

/// Revision ID
#if FF_DEFINED != 80286
#    error Wrong include file (ff.h).
#endif

/// Limits and boundaries
/// Max size of FAT directory
static constexpr const uint32_t MAX_DIR    = 0x200000;
/// Max size of exFAT directory
static constexpr const uint32_t MAX_DIR_EX = 0x10000000;
/// Max FAT12 clusters (differs from specs, but right for real DOS/Windows
/// behavior)
static constexpr const uint32_t MAX_FAT12  = 0xFF5;
/// Max FAT16 clusters (differs from specs, but right for real DOS/Windows
/// behavior)
static constexpr const uint32_t MAX_FAT16  = 0xFFF5;
/// Max FAT32 clusters (not specified, practical limit)
static constexpr const uint32_t MAX_FAT32  = 0x0FFFFFF5;
/// Max exFAT clusters (differs from specs, implementation limit)
static constexpr const uint32_t MAX_EXFAT  = 0x7FFFFFFD;

/// Character code support macros
#define IsUpper(_c)      ((_c) >= 'A' && (_c) <= 'Z')
#define IsLower(_c)      ((_c) >= 'a' && (_c) <= 'z')
#define IsDigit(_c)      ((_c) >= '0' && (_c) <= '9')
#define IsSeparator(_c)  ((_c) == '/' || (_c) == '\\')
#define IsTerminator(_c) ((UINT)(_c) < (FF_USE_LFN ? ' ' : '!'))
#define IsSurrogate(_c)  ((_c) >= 0xD800 && (_c) <= 0xDFFF)
#define IsSurrogateH(_c) ((_c) >= 0xD800 && (_c) <= 0xDBFF)
#define IsSurrogateL(_c) ((_c) >= 0xDC00 && (_c) <= 0xDFFF)

/// Additional file access control and file status flags for internal use
/// Seek to end of the file on file open
static constexpr const uint32_t FA_SEEKEND         = 0x20;
/// File has been modified
static constexpr const uint32_t FA_MODIFIED        = 0x40;
/// FIL.buf[] needs to be written-back
static constexpr const uint32_t FA_DIRTY           = 0x80;

/// Additional file attribute bits for internal use
/// Volume label
static constexpr const uint32_t AM_VOL             = 0x08;
/// LFN entry
static constexpr const uint32_t AM_LFN             = 0x0F;
/// Mask of defined bits in FAT
static constexpr const uint32_t AM_MASK            = 0x3F;
/// Mask of defined bits in exFAT
static constexpr const uint32_t AM_MASKX           = 0x37;

/// Name status flags in fn[11]
/// Index of the name status byte
static constexpr const uint32_t NSFLAG             = 11;
/// Out of 8.3 format
static constexpr const uint32_t NS_LOSS            = 0x01;
/// Force to create LFN entry
static constexpr const uint32_t NS_LFN             = 0x02;
/// Last segment
static constexpr const uint32_t NS_LAST            = 0x04;
/// Lower case flag (body)
static constexpr const uint32_t NS_BODY            = 0x08;
/// Lower case flag (ext)
static constexpr const uint32_t NS_EXT             = 0x10;
/// Dot entry
static constexpr const uint32_t NS_DOT             = 0x20;
/// Do not find LFN
static constexpr const uint32_t NS_NOLFN           = 0x40;
/// Not followed
static constexpr const uint32_t NS_NONAME          = 0x80;

/// exFAT directory entry types
/// Allocation bitmap
static constexpr const uint32_t ET_BITMAP          = 0x81;
/// Up-case table
static constexpr const uint32_t ET_UPCASE          = 0x82;
/// Volume label
static constexpr const uint32_t ET_VLABEL          = 0x83;
/// File and directory
static constexpr const uint32_t ET_FILEDIR         = 0x85;
/// Stream extension
static constexpr const uint32_t ET_STREAM          = 0xC0;
/// Name extension
static constexpr const uint32_t ET_FILENAME        = 0xC1;

// FatFs refers the FAT structure as simple byte array instead of structure
// member because the C structure is not binary compatible between different
// platforms

/// x86 jump instruction (3-byte)
static constexpr const uint32_t BS_JmpBoot         = 0;
/// OEM name (8-byte)
static constexpr const uint32_t BS_OEMName         = 3;
/// Sector size [byte] (WORD)
static constexpr const uint32_t BPB_BytsPerSec     = 11;
/// Cluster size [sector] (BYTE)
static constexpr const uint32_t BPB_SecPerClus     = 13;
/// Size of reserved area [sector] (WORD)
static constexpr const uint32_t BPB_RsvdSecCnt     = 14;
/// Number of FATs (BYTE)
static constexpr const uint32_t BPB_NumFATs        = 16;
/// Size of root directory area for FAT [entry] (WORD)
static constexpr const uint32_t BPB_RootEntCnt     = 17;
/// Volume size (16-bit) [sector] (WORD)
static constexpr const uint32_t BPB_TotSec16       = 19;
/// Media descriptor byte (BYTE)
static constexpr const uint32_t BPB_Media          = 21;
/// FAT size (16-bit) [sector] (WORD)
static constexpr const uint32_t BPB_FATSz16        = 22;
/// Number of sectors per track for int13h [sector] (WORD)
static constexpr const uint32_t BPB_SecPerTrk      = 24;
/// Number of heads for int13h (WORD)
static constexpr const uint32_t BPB_NumHeads       = 26;
/// Volume offset from top of the drive (DWORD)
static constexpr const uint32_t BPB_HiddSec        = 28;
/// Volume size (32-bit) [sector] (DWORD)
static constexpr const uint32_t BPB_TotSec32       = 32;
/// Physical drive number for int13h (BYTE)
static constexpr const uint32_t BS_DrvNum          = 36;
/// WindowsNT error flag (BYTE)
static constexpr const uint32_t BS_NTres           = 37;
/// Extended boot signature (BYTE)
static constexpr const uint32_t BS_BootSig         = 38;
/// Volume serial number (DWORD)
static constexpr const uint32_t BS_VolID           = 39;
/// Volume label string (8-byte)
static constexpr const uint32_t BS_VolLab          = 43;
/// Filesystem type string (8-byte)
static constexpr const uint32_t BS_FilSysType      = 54;
/// Boot code (448-byte)
static constexpr const uint32_t BS_BootCode        = 62;
/// Signature word (WORD)
static constexpr const uint32_t BS_55AA            = 510;

/// FAT32: FAT size [sector] (DWORD)
static constexpr const uint32_t BPB_FATSz32        = 36;
/// FAT32: Extended flags (WORD)
static constexpr const uint32_t BPB_ExtFlags32     = 40;
/// FAT32: Filesystem version (WORD)
static constexpr const uint32_t BPB_FSVer32        = 42;
/// FAT32: Root directory cluster (DWORD)
static constexpr const uint32_t BPB_RootClus32     = 44;
/// FAT32: Offset of FSINFO sector (WORD)
static constexpr const uint32_t BPB_FSInfo32       = 48;
/// FAT32: Offset of backup boot sector (WORD)
static constexpr const uint32_t BPB_BkBootSec32    = 50;
/// FAT32: Physical drive number for int13h (BYTE)
static constexpr const uint32_t BS_DrvNum32        = 64;
/// FAT32: Error flag (BYTE)
static constexpr const uint32_t BS_NTres32         = 65;
/// FAT32: Extended boot signature (BYTE)
static constexpr const uint32_t BS_BootSig32       = 66;
/// FAT32: Volume serial number (DWORD)
static constexpr const uint32_t BS_VolID32         = 67;
/// FAT32: Volume label string (8-byte)
static constexpr const uint32_t BS_VolLab32        = 71;
/// FAT32: Filesystem type string (8-byte)
static constexpr const uint32_t BS_FilSysType32    = 82;
/// FAT32: Boot code (420-byte)
static constexpr const uint32_t BS_BootCode32      = 90;

/// exFAT: MBZ field (53-byte)
static constexpr const uint32_t BPB_ZeroedEx       = 11;
/// exFAT: Volume offset from top of the drive [sector] (QWORD)
static constexpr const uint32_t BPB_VolOfsEx       = 64;
/// exFAT: Volume size [sector] (QWORD)
static constexpr const uint32_t BPB_TotSecEx       = 72;
/// exFAT: FAT offset from top of the volume [sector] (DWORD)
static constexpr const uint32_t BPB_FatOfsEx       = 80;
/// exFAT: FAT size [sector] (DWORD)
static constexpr const uint32_t BPB_FatSzEx        = 84;
/// exFAT: Data offset from top of the volume [sector] (DWORD)
static constexpr const uint32_t BPB_DataOfsEx      = 88;
/// exFAT: Number of clusters (DWORD)
static constexpr const uint32_t BPB_NumClusEx      = 92;
/// exFAT: Root directory start cluster (DWORD)
static constexpr const uint32_t BPB_RootClusEx     = 96;
/// exFAT: Volume serial number (DWORD)
static constexpr const uint32_t BPB_VolIDEx        = 100;
/// exFAT: Filesystem version (WORD)
static constexpr const uint32_t BPB_FSVerEx        = 104;
/// exFAT: Volume flags (WORD)
static constexpr const uint32_t BPB_VolFlagEx      = 106;
/// exFAT: Log2 of sector size in unit of byte (BYTE)
static constexpr const uint32_t BPB_BytsPerSecEx   = 108;
/// exFAT: Log2 of cluster size in unit of sector (BYTE)
static constexpr const uint32_t BPB_SecPerClusEx   = 109;
/// exFAT: Number of FATs (BYTE)
static constexpr const uint32_t BPB_NumFATsEx      = 110;
/// exFAT: Physical drive number for int13h (BYTE)
static constexpr const uint32_t BPB_DrvNumEx       = 111;
/// exFAT: Percent in use (BYTE)
static constexpr const uint32_t BPB_PercInUseEx    = 112;
/// exFAT: Reserved (7-byte)
static constexpr const uint32_t BPB_RsvdEx         = 113;
/// exFAT: Boot code (390-byte)
static constexpr const uint32_t BS_BootCodeEx      = 120;

/// Short file name (11-byte)
static constexpr const uint32_t DIR_Name           = 0;
/// Attribute (BYTE)
static constexpr const uint32_t DIR_Attr           = 11;
/// Lower case flag (BYTE)
static constexpr const uint32_t DIR_NTres          = 12;
/// Created time sub-second (BYTE)
static constexpr const uint32_t DIR_CrtTime10      = 13;
/// Created time (DWORD)
static constexpr const uint32_t DIR_CrtTime        = 14;
/// Last accessed date (WORD)
static constexpr const uint32_t DIR_LstAccDate     = 18;
/// Higher 16-bit of first cluster (WORD)
static constexpr const uint32_t DIR_FstClusHI      = 20;
/// Modified time (DWORD)
static constexpr const uint32_t DIR_ModTime        = 22;
/// Lower 16-bit of first cluster (WORD)
static constexpr const uint32_t DIR_FstClusLO      = 26;
/// File size (DWORD)
static constexpr const uint32_t DIR_FileSize       = 28;
/// LFN: LFN order and LLE flag (BYTE)
static constexpr const uint32_t LDIR_Ord           = 0;
/// LFN: LFN attribute (BYTE)
static constexpr const uint32_t LDIR_Attr          = 11;
/// LFN: Entry type (BYTE)
static constexpr const uint32_t LDIR_Type          = 12;
/// LFN: Checksum of the SFN (BYTE)
static constexpr const uint32_t LDIR_Chksum        = 13;
/// LFN: MBZ field (WORD)
static constexpr const uint32_t LDIR_FstClusLO     = 26;
/// exFAT: Type of exFAT directory entry (BYTE)
static constexpr const uint32_t XDIR_Type          = 0;
/// exFAT: Number of volume label characters (BYTE)
static constexpr const uint32_t XDIR_NumLabel      = 1;
/// exFAT: Volume label (11-WORD)
static constexpr const uint32_t XDIR_Label         = 2;
/// exFAT: Sum of case conversion table (DWORD)
static constexpr const uint32_t XDIR_CaseSum       = 4;
/// exFAT: Number of secondary entries (BYTE)
static constexpr const uint32_t XDIR_NumSec        = 1;
/// exFAT: Sum of the set of directory entries (WORD)
static constexpr const uint32_t XDIR_SetSum        = 2;
/// exFAT: File attribute (WORD)
static constexpr const uint32_t XDIR_Attr          = 4;
/// exFAT: Created time (DWORD)
static constexpr const uint32_t XDIR_CrtTime       = 8;
/// exFAT: Modified time (DWORD)
static constexpr const uint32_t XDIR_ModTime       = 12;
/// exFAT: Last accessed time (DWORD)
static constexpr const uint32_t XDIR_AccTime       = 16;
/// exFAT: Created time subsecond (BYTE)
static constexpr const uint32_t XDIR_CrtTime10     = 20;
/// exFAT: Modified time subsecond (BYTE)
static constexpr const uint32_t XDIR_ModTime10     = 21;
/// exFAT: Created timezone (BYTE)
static constexpr const uint32_t XDIR_CrtTZ         = 22;
/// exFAT: Modified timezone (BYTE)
static constexpr const uint32_t XDIR_ModTZ         = 23;
/// exFAT: Last accessed timezone (BYTE)
static constexpr const uint32_t XDIR_AccTZ         = 24;
/// exFAT: General secondary flags (BYTE)
static constexpr const uint32_t XDIR_GenFlags      = 33;
/// exFAT: Number of file name characters (BYTE)
static constexpr const uint32_t XDIR_NumName       = 35;
/// exFAT: Hash of file name (WORD)
static constexpr const uint32_t XDIR_NameHash      = 36;
/// exFAT: Valid file size (QWORD)
static constexpr const uint32_t XDIR_ValidFileSize = 40;
/// exFAT: First cluster of the file data (DWORD)
static constexpr const uint32_t XDIR_FstClus       = 52;
/// exFAT: File/Directory size (QWORD)
static constexpr const uint32_t XDIR_FileSize      = 56;

/// Size of a directory entry
static constexpr const uint32_t SZDIRE             = 32;
/// Deleted directory entry mark set to DIR_Name[0]
static constexpr const uint32_t DDEM               = 0xE5;
/// Replacement of the character collides with DDEM
static constexpr const uint32_t RDDEM              = 0x05;
/// Last long entry flag in LDIR_Ord
static constexpr const uint32_t LLEF               = 0x40;

/// FAT32 FSI: Leading signature (DWORD)
static constexpr const uint32_t FSI_LeadSig        = 0;
/// FAT32 FSI: Structure signature (DWORD)
static constexpr const uint32_t FSI_StrucSig       = 484;
/// FAT32 FSI: Number of free clusters (DWORD)
static constexpr const uint32_t FSI_Free_Count     = 488;
/// FAT32 FSI: Last allocated cluster (DWORD)
static constexpr const uint32_t FSI_Nxt_Free       = 492;

/// MBR: Offset of partition table in the MBR
static constexpr const uint32_t MBR_Table          = 446;
/// MBR: Size of a partition table entry
static constexpr const uint32_t SZ_PTE             = 16;
/// MBR PTE: Boot indicator
static constexpr const uint32_t PTE_Boot           = 0;
/// MBR PTE: Start head
static constexpr const uint32_t PTE_StHead         = 1;
/// MBR PTE: Start sector
static constexpr const uint32_t PTE_StSec          = 2;
/// MBR PTE: Start cylinder
static constexpr const uint32_t PTE_StCyl          = 3;
/// MBR PTE: System ID
static constexpr const uint32_t PTE_System         = 4;
/// MBR PTE: End head
static constexpr const uint32_t PTE_EdHead         = 5;
/// MBR PTE: End sector
static constexpr const uint32_t PTE_EdSec          = 6;
/// MBR PTE: End cylinder
static constexpr const uint32_t PTE_EdCyl          = 7;
/// MBR PTE: Start in LBA
static constexpr const uint32_t PTE_StLba          = 8;
/// MBR PTE: Size in LBA
static constexpr const uint32_t PTE_SizLba         = 12;

/// GPT HDR: Signature (8-byte)
static constexpr const uint32_t GPTH_Sign          = 0;
/// GPT HDR: Revision (DWORD)
static constexpr const uint32_t GPTH_Rev           = 8;
/// GPT HDR: Header size (DWORD)
static constexpr const uint32_t GPTH_Size          = 12;
/// GPT HDR: Header BCC (DWORD)
static constexpr const uint32_t GPTH_Bcc           = 16;
/// GPT HDR: This header LBA (QWORD)
static constexpr const uint32_t GPTH_CurLba        = 24;
/// GPT HDR: Another header LBA (QWORD)
static constexpr const uint32_t GPTH_BakLba        = 32;
/// GPT HDR: First LBA for partition data (QWORD)
static constexpr const uint32_t GPTH_FstLba        = 40;
/// GPT HDR: Last LBA for partition data (QWORD)
static constexpr const uint32_t GPTH_LstLba        = 48;
/// GPT HDR: Disk GUID (16-byte)
static constexpr const uint32_t GPTH_DskGuid       = 56;
/// GPT HDR: Partition table LBA (QWORD)
static constexpr const uint32_t GPTH_PtOfs         = 72;
/// GPT HDR: Number of table entries (DWORD)
static constexpr const uint32_t GPTH_PtNum         = 80;
/// GPT HDR: Size of table entry (DWORD)
static constexpr const uint32_t GPTH_PteSize       = 84;
/// GPT HDR: Partition table BCC (DWORD)
static constexpr const uint32_t GPTH_PtBcc         = 88;
/// GPT PTE: Size of partition table entry
static constexpr const uint32_t SZ_GPTE            = 128;
/// GPT PTE: Partition type GUID (16-byte)
static constexpr const uint32_t GPTE_PtGuid        = 0;
/// GPT PTE: Partition unique GUID (16-byte)
static constexpr const uint32_t GPTE_UpGuid        = 16;
/// GPT PTE: First LBA of partition (QWORD)
static constexpr const uint32_t GPTE_FstLba        = 32;
/// GPT PTE: Last LBA of partition (QWORD)
static constexpr const uint32_t GPTE_LstLba        = 40;
/// GPT PTE: Partition flags (QWORD)
static constexpr const uint32_t GPTE_Flags         = 48;
/// GPT PTE: Partition name
static constexpr const uint32_t GPTE_Name          = 56;

/// Post process on fatal error in the file operations
#define ABORT(_fs, _res)        \
    {                           \
        fp->err = (BYTE)(_res); \
        LEAVE_FF(_fs, _res);    \
    }

/// Re-entrancy related
#if FF_FS_REENTRANT
#    if FF_USE_LFN == 1
#        error Static LFN work area cannot be used in thread-safe configuration
#    endif
#    define LEAVE_FF(_fs, _res)       \
        {                             \
            unlock_volume(_fs, _res); \
            return _res;              \
        }
#else
#    define LEAVE_FF(_fs, _res) return _res
#endif

/// Definitions of logical drive - physical location conversion
#if FF_MULTI_PARTITION
/// Get physical drive number
#    define LD2PD(_vol) VolToPart[_vol].pd
/// Get partition number (0:auto search, 1..:forced partition number)
#    define LD2PT(_vol) VolToPart[_vol].pt
#else
/// Each logical drive is associated with the same physical drive number
#    define LD2PD(_vol) (BYTE)(_vol)
/// Auto partition search
#    define LD2PT(_vol) 0
#endif

/// Definitions of sector size
#if (FF_MAX_SS < FF_MIN_SS)                                      \
  || (FF_MAX_SS != 512 && FF_MAX_SS != 1024 && FF_MAX_SS != 2048 \
      && FF_MAX_SS != 4096)                                      \
  || (FF_MIN_SS != 512 && FF_MIN_SS != 1024 && FF_MIN_SS != 2048 \
      && FF_MIN_SS != 4096)
#    error Wrong sector size configuration
#endif
#if FF_MAX_SS == FF_MIN_SS
/// Fixed sector size
#    define SS(_fs) ((UINT)FF_MAX_SS)
#else
/// Variable sector size
#    define SS(_fs) ((_fs)->ssize)
#endif

/// Timestamp
#if FF_FS_NORTC == 1
#    if FF_NORTC_YEAR < 1980 || FF_NORTC_YEAR > 2107 || FF_NORTC_MON < 1 \
      || FF_NORTC_MON > 12 || FF_NORTC_MDAY < 1 || FF_NORTC_MDAY > 31
#        error Invalid FF_FS_NORTC settings
#    endif
#    define GET_FATTIME()                                                \
        ((DWORD)(FF_NORTC_YEAR - 1980) << 25 | (DWORD)FF_NORTC_MON << 21 \
         | (DWORD)FF_NORTC_MDAY << 16)
#else
#    define GET_FATTIME() get_fattime()
#endif

/// File lock controls
#if FF_FS_LOCK
#    if FF_FS_READONLY
#        error FF_FS_LOCK must be 0 at read-only configuration
#    endif
struct FILESEM {
    /// Object ID 1, volume (NULL:blank entry)
    FATFS* fs;
    /// Object ID 2, containing directory (0:root)
    DWORD  clu;
    /// Object ID 3, offset in the directory
    DWORD  ofs;
    /// Object open counter, 0:none, 0x01..0xFF:read mode open count,
    /// 0x100:write mode
    UINT   ctr;
};
#endif

/// SBCS up-case tables (\x80-\xFF)
static constexpr const uint8_t TBL_CT437[]
  = { 0x80, 0x9A, 0x45, 0x41, 0x8E, 0x41, 0x8F, 0x80, 0x45, 0x45, 0x45, 0x49,
      0x49, 0x49, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0x4F, 0x99, 0x4F, 0x55, 0x55,
      0x59, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x41, 0x49, 0x4F, 0x55,
      0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT720[]
  = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3,
      0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT737[]
  = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0xAA, 0x92, 0x93, 0x94, 0x95, 0x96,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0x97, 0xEA, 0xEB, 0xEC, 0xE4, 0xED, 0xEE, 0xEF, 0xF5, 0xF0, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT771[]
  = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x80, 0x81, 0x82, 0x83,
      0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDC, 0xDE, 0xDE,
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
      0x9C, 0x9D, 0x9E, 0x9F, 0xF0, 0xF0, 0xF2, 0xF2, 0xF4, 0xF4, 0xF6, 0xF6,
      0xF8, 0xF8, 0xFA, 0xFA, 0xFC, 0xFC, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT775[]
  = { 0x80, 0x9A, 0x91, 0xA0, 0x8E, 0x95, 0x8F, 0x80, 0xAD, 0xED, 0x8A, 0x8A,
      0xA1, 0x8D, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0xE2, 0x99, 0x95, 0x96, 0x97,
      0x97, 0x99, 0x9A, 0x9D, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xE0, 0xA3,
      0xA3, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xB5, 0xB6, 0xB7, 0xB8,
      0xBD, 0xBE, 0xC6, 0xC7, 0xA5, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE5, 0xE5, 0xE6, 0xE3, 0xE8, 0xE8, 0xEA, 0xEA,
      0xEE, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT850[]
  = { 0x43, 0x55, 0x45, 0x41, 0x41, 0x41, 0x41, 0x43, 0x45, 0x45, 0x45, 0x49,
      0x49, 0x49, 0x41, 0x41, 0x45, 0x92, 0x92, 0x4F, 0x4F, 0x4F, 0x55, 0x55,
      0x59, 0x4F, 0x55, 0x4F, 0x9C, 0x4F, 0x9E, 0x9F, 0x41, 0x49, 0x4F, 0x55,
      0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0x41, 0x41, 0x41, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0x41, 0x41,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD1, 0xD1, 0x45, 0x45,
      0x45, 0x49, 0x49, 0x49, 0x49, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0x49, 0xDF,
      0x4F, 0xE1, 0x4F, 0x4F, 0x4F, 0x4F, 0xE6, 0xE8, 0xE8, 0x55, 0x55, 0x55,
      0x59, 0x59, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT852[]
  = { 0x80, 0x9A, 0x90, 0xB6, 0x8E, 0xDE, 0x8F, 0x80, 0x9D, 0xD3, 0x8A, 0x8A,
      0xD7, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x91, 0xE2, 0x99, 0x95, 0x95, 0x97,
      0x97, 0x99, 0x9A, 0x9B, 0x9B, 0x9D, 0x9E, 0xAC, 0xB5, 0xD6, 0xE0, 0xE9,
      0xA4, 0xA4, 0xA6, 0xA6, 0xA8, 0xA8, 0xAA, 0x8D, 0xAC, 0xB8, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBD, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC6,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD1, 0xD1, 0xD2, 0xD3,
      0xD2, 0xD5, 0xD6, 0xD7, 0xB7, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE3, 0xD5, 0xE6, 0xE6, 0xE8, 0xE9, 0xE8, 0xEB,
      0xED, 0xED, 0xDD, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xEB, 0xFC, 0xFC, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT855[]
  = { 0x81, 0x81, 0x83, 0x83, 0x85, 0x85, 0x87, 0x87, 0x89, 0x89, 0x8B, 0x8B,
      0x8D, 0x8D, 0x8F, 0x8F, 0x91, 0x91, 0x93, 0x93, 0x95, 0x95, 0x97, 0x97,
      0x99, 0x99, 0x9B, 0x9B, 0x9D, 0x9D, 0x9F, 0x9F, 0xA1, 0xA1, 0xA3, 0xA3,
      0xA5, 0xA5, 0xA7, 0xA7, 0xA9, 0xA9, 0xAB, 0xAB, 0xAD, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB6, 0xB6, 0xB8, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBE, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC7, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD1, 0xD1, 0xD3, 0xD3,
      0xD5, 0xD5, 0xD7, 0xD7, 0xDD, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xE0, 0xDF,
      0xE0, 0xE2, 0xE2, 0xE4, 0xE4, 0xE6, 0xE6, 0xE8, 0xE8, 0xEA, 0xEA, 0xEC,
      0xEC, 0xEE, 0xEE, 0xEF, 0xF0, 0xF2, 0xF2, 0xF4, 0xF4, 0xF6, 0xF6, 0xF8,
      0xF8, 0xFA, 0xFA, 0xFC, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT857[]
  = { 0x80, 0x9A, 0x90, 0xB6, 0x8E, 0xB7, 0x8F, 0x80, 0xD2, 0xD3, 0xD4, 0xD8,
      0xD7, 0x49, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0xE2, 0x99, 0xE3, 0xEA, 0xEB,
      0x98, 0x99, 0x9A, 0x9D, 0x9C, 0x9D, 0x9E, 0x9E, 0xB5, 0xD6, 0xE0, 0xE9,
      0xA5, 0xA5, 0xA6, 0xA6, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC7, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0x49, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE5, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xDE, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT860[]
  = { 0x80, 0x9A, 0x90, 0x8F, 0x8E, 0x91, 0x86, 0x80, 0x89, 0x89, 0x92, 0x8B,
      0x8C, 0x98, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x8C, 0x99, 0xA9, 0x96, 0x9D,
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x86, 0x8B, 0x9F, 0x96,
      0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT861[]
  = { 0x80, 0x9A, 0x90, 0x41, 0x8E, 0x41, 0x8F, 0x80, 0x45, 0x45, 0x45, 0x8B,
      0x8B, 0x8D, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0x4F, 0x99, 0x8D, 0x55, 0x97,
      0x97, 0x99, 0x9A, 0x9D, 0x9C, 0x9D, 0x9E, 0x9F, 0xA4, 0xA5, 0xA6, 0xA7,
      0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT862[]
  = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x41, 0x49, 0x4F, 0x55,
      0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT863[]
  = { 0x43, 0x55, 0x45, 0x41, 0x41, 0x41, 0x86, 0x43, 0x45, 0x45, 0x45, 0x49,
      0x49, 0x8D, 0x41, 0x8F, 0x45, 0x45, 0x45, 0x4F, 0x45, 0x49, 0x55, 0x55,
      0x98, 0x4F, 0x55, 0x9B, 0x9C, 0x55, 0x55, 0x9F, 0xA0, 0xA1, 0x4F, 0x55,
      0xA4, 0xA5, 0xA6, 0xA7, 0x49, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT864[]
  = { 0x80, 0x9A, 0x45, 0x41, 0x8E, 0x41, 0x8F, 0x80, 0x45, 0x45, 0x45, 0x49,
      0x49, 0x49, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0x4F, 0x99, 0x4F, 0x55, 0x55,
      0x59, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x41, 0x49, 0x4F, 0x55,
      0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT865[]
  = { 0x80, 0x9A, 0x90, 0x41, 0x8E, 0x41, 0x8F, 0x80, 0x45, 0x45, 0x45, 0x49,
      0x49, 0x49, 0x8E, 0x8F, 0x90, 0x92, 0x92, 0x4F, 0x99, 0x4F, 0x55, 0x55,
      0x59, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x41, 0x49, 0x4F, 0x55,
      0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
      0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT866[]
  = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x80, 0x81, 0x82, 0x83,
      0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
      0x9C, 0x9D, 0x9E, 0x9F, 0xF0, 0xF0, 0xF2, 0xF2, 0xF4, 0xF4, 0xF6, 0xF6,
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static constexpr const uint8_t TBL_CT869[]
  = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
      0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x98, 0x99, 0x9A, 0x86, 0x9C, 0x8D, 0x8F, 0x90, 0x91, 0x90, 0x92, 0x95,
      0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
      0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
      0xD4, 0xD5, 0xA4, 0xA5, 0xA6, 0xD9, 0xDA, 0xDB, 0xDC, 0xA7, 0xA8, 0xDF,
      0xA9, 0xAA, 0xAC, 0xAD, 0xB5, 0xB6, 0xB7, 0xB8, 0xBD, 0xBE, 0xC6, 0xC7,
      0xCF, 0xCF, 0xD0, 0xEF, 0xF0, 0xF1, 0xD1, 0xD2, 0xD3, 0xF5, 0xD4, 0xF7,
      0xF8, 0xF9, 0xD5, 0x96, 0x95, 0x98, 0xFE, 0xFF };

/* DBCS code range |----- 1st byte -----|  |----------- 2nd byte -----------| */
/*                  <------>    <------>    <------>    <------>    <------>  */
static constexpr const uint8_t TBL_DC932[]
  = { 0x81, 0x9F, 0xE0, 0xFC, 0x40, 0x7E, 0x80, 0xFC, 0x00, 0x00 };
static constexpr const uint8_t TBL_DC936[]
  = { 0x81, 0xFE, 0x00, 0x00, 0x40, 0x7E, 0x80, 0xFE, 0x00, 0x00 };
static constexpr const uint8_t TBL_DC949[]
  = { 0x81, 0xFE, 0x00, 0x00, 0x41, 0x5A, 0x61, 0x7A, 0x81, 0xFE };
static constexpr const uint8_t TBL_DC950[]
  = { 0x81, 0xFE, 0x00, 0x00, 0x40, 0x7E, 0xA1, 0xFE, 0x00, 0x00 };

/// Macros for table definitions
#define MERGE_2STR(_a, _b) _a##_b
#define MKCVTBL(_hd, _cp)  MERGE_2STR(_hd, _cp)

/*--------------------------------------------------------------------------

   Module Private Work Area

---------------------------------------------------------------------------*/
/* Remark: Variables defined here without initial value shall be guaranteed
/  zero/null at start-up. If not, the linker option or start-up routine is
/  not compliance with C standard. */

/*--------------------------------*/
/* File/Volume controls           */
/*--------------------------------*/

#if FF_VOLUMES < 1 || FF_VOLUMES > 10
#    error Wrong FF_VOLUMES setting
#endif
/// Pointer to the filesystem objects (logical drives)
static FATFS* FatFs[FF_VOLUMES];
/// Filesystem mount ID
static WORD   Fsid;

#if FF_FS_RPATH != 0
/// Current drive set by f_chdrive()
static BYTE CurrVol;
#endif

#if FF_FS_LOCK != 0
/// Open object lock semaphores
static FILESEM Files[FF_FS_LOCK];
#    if FF_FS_REENTRANT
/// System lock flag (0:no mutex, 1:unlocked, 2:locked)
static BYTE SysLock;
#    endif
#endif

#if FF_STR_VOLUME_ID
#    ifdef FF_VOLUME_STRS
/// Pre-defined volume ID
static const char* const VolumeStr[FF_VOLUMES] = { FF_VOLUME_STRS };
#    endif
#endif

#if FF_LBA64
#    if FF_MIN_GPT > 0x100000000
#        error Wrong FF_MIN_GPT setting
#    endif
static const BYTE GUID_MS_Basic[16]
  = { 0xA2, 0xA0, 0xD0, 0xEB, 0xE5, 0xB9, 0x33, 0x44,
      0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 };
#endif

/*--------------------------------*/
/* LFN/Directory working buffer   */
/*--------------------------------*/

/// Non-LFN configuration
#if FF_USE_LFN == 0
#    if FF_FS_EXFAT
#        error LFN must be enabled when enable exFAT
#    endif
#    define DEF_NAMBUF
#    define INIT_NAMBUF(_fs)
#    define FREE_NAMBUF()
#    define LEAVE_MKFS(_res) return _res

/// LFN configurations
#else
#    if FF_MAX_LFN < 12 || FF_MAX_LFN > 255
#        error Wrong setting of FF_MAX_LFN
#    endif
#    if FF_LFN_BUF < FF_SFN_BUF || FF_SFN_BUF < 12
#        error Wrong setting of FF_LFN_BUF or FF_SFN_BUF
#    endif
#    if FF_LFN_UNICODE < 0 || FF_LFN_UNICODE > 3
#        error Wrong setting of FF_LFN_UNICODE
#    endif
/// FAT: Offset of LFN characters in the directory entry
static const BYTE LfnOfs[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
/// exFAT: Size of directory entry block scratchpad buffer needed for the name
/// length
#    define MAXDIRB(_nc) ((_nc + 44U) / 15 * SZDIRE)

/// LFN enabled with static working buffer
#    if FF_USE_LFN == 1
#        if FF_FS_EXFAT
/// Directory entry block scratchpad buffer
static BYTE       DirBuf[MAXDIRB(FF_MAX_LFN)];
#        endif
/// LFN working buffer
static WCHAR      LfnBuf[FF_MAX_LFN + 1];
#        define DEF_NAMBUF
#        define INIT_NAMBUF(_fs)
#        define FREE_NAMBUF()
#        define LEAVE_MKFS(_res) return _res

/// LFN enabled with dynamic working buffer on the stack
#    elif FF_USE_LFN == 2
#        if FF_FS_EXFAT
/// LFN working buffer and directory entry block scratchpad buffer
#            define DEF_NAMBUF              \
                WCHAR lbuf[FF_MAX_LFN + 1]; \
                BYTE  dbuf[MAXDIRB(FF_MAX_LFN)];
#            define INIT_NAMBUF(_fs)      \
                {                         \
                    (_fs)->lfnbuf = lbuf; \
                    (_fs)->dirbuf = dbuf; \
                }
#            define FREE_NAMBUF()
#        else
/// LFN working buffer
#            define DEF_NAMBUF WCHAR lbuf[FF_MAX_LFN + 1];
#            define INIT_NAMBUF(_fs) \
                { (_fs)->lfnbuf = lbuf; }
#            define FREE_NAMBUF()
#        endif
#        define LEAVE_MKFS(_res) return _res

/// LFN enabled with dynamic working buffer on the heap
#    elif FF_USE_LFN == 3
#        if FF_FS_EXFAT
/// Pointer to LFN working buffer and directory entry block scratchpad buffer
#            define DEF_NAMBUF WCHAR* lfn;
#            define INIT_NAMBUF(_fs)                               \
                {                                                  \
                    lfn = ff_memalloc((FF_MAX_LFN + 1) * 2         \
                                      + MAXDIRB(FF_MAX_LFN));      \
                    if (!lfn)                                      \
                        LEAVE_FF(_fs, FR_NOT_ENOUGH_CORE);         \
                    (_fs)->lfnbuf = lfn;                           \
                    (_fs)->dirbuf = (BYTE*)(lfn + FF_MAX_LFN + 1); \
                }
#            define FREE_NAMBUF() ff_memfree(lfn)
#        else
/// Pointer to LFN working buffer
#            define DEF_NAMBUF WCHAR* lfn;
#            define INIT_NAMBUF(_fs)                         \
                {                                            \
                    lfn = ff_memalloc((FF_MAX_LFN + 1) * 2); \
                    if (!lfn)                                \
                        LEAVE_FF(_fs, FR_NOT_ENOUGH_CORE);   \
                    (_fs)->lfnbuf = lfn;                     \
                }
#            define FREE_NAMBUF() ff_memfree(lfn)
#        endif
#        define LEAVE_MKFS(_res)     \
            {                        \
                if (!work)           \
                    ff_memfree(buf); \
                return _res;         \
            }
/// Must be >=FF_MAX_SS
static constexpr const uint32_t MAX_MALLOC = 0x8000;

#    else
#        error Wrong setting of FF_USE_LFN

/// FF_USE_LFN == 1
#    endif
/// FF_USE_LFN == 0
#endif

/*--------------------------------*/
/* Code conversion tables         */
/*--------------------------------*/

/// Run-time code page configuration
#if FF_CODE_PAGE == 0
#    define CODEPAGE CodePage
/// Current code page
static WORD        CodePage;
/// Ptr to SBCS up-case table Ct???[] (null:not used)
static const BYTE* ExCvt;
/// Ptr to DBCS code range table Dc???[] (null:not used)
static const BYTE* DbcTbl;

static const BYTE  Ct437[] = TBL_CT437;
static const BYTE  Ct720[] = TBL_CT720;
static const BYTE  Ct737[] = TBL_CT737;
static const BYTE  Ct771[] = TBL_CT771;
static const BYTE  Ct775[] = TBL_CT775;
static const BYTE  Ct850[] = TBL_CT850;
static const BYTE  Ct852[] = TBL_CT852;
static const BYTE  Ct855[] = TBL_CT855;
static const BYTE  Ct857[] = TBL_CT857;
static const BYTE  Ct860[] = TBL_CT860;
static const BYTE  Ct861[] = TBL_CT861;
static const BYTE  Ct862[] = TBL_CT862;
static const BYTE  Ct863[] = TBL_CT863;
static const BYTE  Ct864[] = TBL_CT864;
static const BYTE  Ct865[] = TBL_CT865;
static const BYTE  Ct866[] = TBL_CT866;
static const BYTE  Ct869[] = TBL_CT869;
static const BYTE  Dc932[] = TBL_DC932;
static const BYTE  Dc936[] = TBL_DC936;
static const BYTE  Dc949[] = TBL_DC949;
static const BYTE  Dc950[] = TBL_DC950;

/// Static code page configuration (SBCS)
#elif FF_CODE_PAGE < 900
#    define CODEPAGE FF_CODE_PAGE
static constexpr const auto ExCvt = MKCVTBL(TBL_CT, FF_CODE_PAGE);
/// Static code page configuration (DBCS)
#else
#    define CODEPAGE FF_CODE_PAGE
static constexpr const auto DbcTbl = MKCVTBL(TBL_DC, FF_CODE_PAGE);

#endif

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Load/Store multi-byte word in the FAT structure                       */
/*-----------------------------------------------------------------------*/

///	 Load a 2-byte little-endian word
static WORD ld_word(const BYTE* ptr) {
    WORD rv;

    rv = ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

/// Load a 4-byte little-endian word
static DWORD ld_dword(const BYTE* ptr) {
    DWORD rv;

    rv = ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

#if FF_FS_EXFAT
/// Load an 8-byte little-endian word
static QWORD ld_qword(const BYTE* ptr) {
    QWORD rv;

    rv = ptr[7];
    rv = rv << 8 | ptr[6];
    rv = rv << 8 | ptr[5];
    rv = rv << 8 | ptr[4];
    rv = rv << 8 | ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}
#endif

#if !FF_FS_READONLY
/// Store a 2-byte word in little-endian
static void st_word(BYTE* ptr, WORD val) {
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
}

/// Store a 4-byte word in little-endian
static void st_dword(BYTE* ptr, DWORD val) {
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
}

/// Store an 8-byte word in little-endian
#    if FF_FS_EXFAT
static void st_qword(BYTE* ptr, QWORD val) {
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
    val    >>= 8;
    *ptr++   = (BYTE)val;
}
#    endif

#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/// Test if the byte is DBC 1st byte

static int dbc_1st(BYTE c) {
    /// Variable code page
#if FF_CODE_PAGE == 0
    if (DbcTbl && c >= DbcTbl[0]) {
        /// 1st byte range 1
        if (c <= DbcTbl[1]) {
            return 1;
        }
        /// 1st byte range 2
        if (c >= DbcTbl[2] && c <= DbcTbl[3]) {
            return 1;
        }
    }
    /// DBCS fixed code page
#elif FF_CODE_PAGE >= 900
    if (c >= DbcTbl[0]) {
        if (c <= DbcTbl[1]) {
            return 1;
        }
        if (c >= DbcTbl[2] && c <= DbcTbl[3]) {
            return 1;
        }
    }
    /// SBCS fixed code page
#else
    /// Always false
    if (c != 0) {
        return 0;
    }
#endif
    return 0;
}

/// Test if the byte is DBC 2nd byte

static int dbc_2nd(BYTE c) {
    /// Variable code page
#if FF_CODE_PAGE == 0
    if (DbcTbl && c >= DbcTbl[4]) {
        /// 2nd byte range 1
        if (c <= DbcTbl[5]) {
            return 1;
        }
        /// 2nd byte range 2
        if (c >= DbcTbl[6] && c <= DbcTbl[7]) {
            return 1;
        }
        /// 2nd byte range 3
        if (c >= DbcTbl[8] && c <= DbcTbl[9]) {
            return 1;
        }
    }
    /// DBCS fixed code page
#elif FF_CODE_PAGE >= 900
    if (c >= DbcTbl[4]) {
        if (c <= DbcTbl[5]) {
            return 1;
        }
        if (c >= DbcTbl[6] && c <= DbcTbl[7]) {
            return 1;
        }
        if (c >= DbcTbl[8] && c <= DbcTbl[9]) {
            return 1;
        }
    }
    /// SBCS fixed code page
#else
    /// Always false
    if (c != 0) {
        return 0;
    }
#endif
    return 0;
}

#if FF_USE_LFN

/// Get a Unicode code point from the TCHAR string in defined API encodeing

/// Returns a character in UTF-16 encoding (>=0x10000 on surrogate pair,
/// 0xFFFFFFFF on decode error)
static DWORD tchar2uni(
  /// Pointer to pointer to TCHAR string in configured encoding
  const TCHAR** str) {
    DWORD        uc;
    const TCHAR* p = *str;

/// UTF-16 input
#    if FF_LFN_UNICODE == 1
    WCHAR wc;

    /// Get a unit
    uc = *p++;
    /// Surrogate?
    if (IsSurrogate(uc)) {
        /// Get low surrogate
        wc = *p++;
        /// Wrong surrogate?
        if (!IsSurrogateH(uc) || !IsSurrogateL(wc)) {
            return 0xFFFFFFFF;
        }
        uc = uc << 16 | wc;
    }

/// UTF-8 input
#    elif FF_LFN_UNICODE == 2
    BYTE b;
    int  nf;

    /// Get an encoding unit
    uc = (BYTE)*p++;
    /// Multiple byte code?
    if (uc & 0x80) {
        /// 2-byte sequence?
        if ((uc & 0xE0) == 0xC0) {
            uc &= 0x1F;
            nf  = 1;
            /// 3-byte sequence?
        }
        else if ((uc & 0xF0) == 0xE0) {
            uc &= 0x0F;
            nf  = 2;
            /// 4-byte sequence?
        }
        else if ((uc & 0xF8) == 0xF0) {
            uc &= 0x07;
            nf  = 3;
            /// Wrong sequence
        }
        else {
            return 0xFFFFFFFF;
        }
        /// Get trailing bytes
        do {
            b = (BYTE)*p++;
            /// Wrong sequence?
            if ((b & 0xC0) != 0x80) {
                return 0xFFFFFFFF;
            }
            uc = uc << 6 | (b & 0x3F);
        } while (--nf != 0);
        /// Wrong code?
        if (uc < 0x80 || IsSurrogate(uc) || uc >= 0x110000) {
            return 0xFFFFFFFF;
        }
        /// Make a surrogate pair if needed
        if (uc >= 0x010000) {
            uc = 0xD800DC00 | ((uc - 0x10000) << 6 & 0x3FF0000) | (uc & 0x3FF);
        }
    }

/// UTF-32 input
#    elif FF_LFN_UNICODE == 3
    /// Get a unit
    uc = (TCHAR)*p++;
    /// Wrong code?
    if (uc >= 0x110000 || IsSurrogate(uc)) {
        return 0xFFFFFFFF;
    }
    /// Make a surrogate pair if needed
    if (uc >= 0x010000) {
        uc = 0xD800DC00 | ((uc - 0x10000) << 6 & 0x3FF0000) | (uc & 0x3FF);
    }

/// ANSI/OEM input
#    else
    BYTE  b;
    WCHAR wc;

    /// Get a byte
    wc = (BYTE)*p++;
    /// Is it a DBC 1st byte?
    if (dbc_1st((BYTE)wc)) {
        /// Get 2nd byte
        b = (BYTE)*p++;
        /// Invalid code?
        if (!dbc_2nd(b)) {
            return 0xFFFFFFFF;
        }
        /// Make a DBC
        wc = (wc << 8) + b;
    }
    if (wc != 0) {
        /// ANSI/OEM ==> Unicode
        wc = ff_oem2uni(wc, CODEPAGE);
        /// Invalid code?
        if (wc == 0) {
            return 0xFFFFFFFF;
        }
    }
    uc = wc;

#    endif
    /// Next read pointer
    *str = p;
    return uc;
}

/// Store a Unicode char in defined API encoding

/// Returns number of encoding units written (0:buffer overflow or wrong
/// encoding)
static UINT put_utf(
  /// UTF-16 encoded character (Surrogate pair if >=0x10000)
  DWORD  chr,
  /// Output buffer
  TCHAR* buf,
  /// Size of the buffer
  UINT   szb) {
    /// UTF-16 output
#    if FF_LFN_UNICODE == 1
    WCHAR hs, wc;

    hs = (WCHAR)(chr >> 16);
    wc = (WCHAR)chr;
    /// Single encoding unit?
    if (hs == 0) {
        /// Buffer overflow or wrong code?
        if (szb < 1 || IsSurrogate(wc)) {
            return 0;
        }
        *buf = wc;
        return 1;
    }
    /// Buffer overflow or wrong surrogate?
    if (szb < 2 || !IsSurrogateH(hs) || !IsSurrogateL(wc)) {
        return 0;
    }
    *buf++ = hs;
    *buf++ = wc;
    return 2;

/// UTF-8 output
#    elif FF_LFN_UNICODE == 2
    DWORD hc;

    /// Single byte code?
    if (chr < 0x80) {
        /// Buffer overflow?
        if (szb < 1) {
            return 0;
        }
        *buf = (TCHAR)chr;
        return 1;
    }
    /// 2-byte sequence?
    if (chr < 0x800) {
        /// Buffer overflow?
        if (szb < 2) {
            return 0;
        }
        *buf++ = (TCHAR)(0xC0 | (chr >> 6 & 0x1F));
        *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
        return 2;
    }
    /// 3-byte sequence?
    if (chr < 0x10000) {
        /// Buffer overflow or wrong code?
        if (szb < 3 || IsSurrogate(chr)) {
            return 0;
        }
        *buf++ = (TCHAR)(0xE0 | (chr >> 12 & 0x0F));
        *buf++ = (TCHAR)(0x80 | (chr >> 6 & 0x3F));
        *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
        return 3;
    }
    /// 4-byte sequence

    /// Buffer overflow?
    if (szb < 4) {
        return 0;
    }
    /// Get high 10 bits
    hc  = ((chr & 0xFFFF0000) - 0xD8000000) >> 6;
    /// Get low 10 bits
    chr = (chr & 0xFFFF) - 0xDC00;
    /// Wrong surrogate?
    if (hc >= 0x100000 || chr >= 0x400) {
        return 0;
    }
    chr    = (hc | chr) + 0x10000;
    *buf++ = (TCHAR)(0xF0 | (chr >> 18 & 0x07));
    *buf++ = (TCHAR)(0x80 | (chr >> 12 & 0x3F));
    *buf++ = (TCHAR)(0x80 | (chr >> 6 & 0x3F));
    *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
    return 4;

/// UTF-32 output
#    elif FF_LFN_UNICODE == 3
    DWORD hc;

    /// Buffer overflow?
    if (szb < 1) {
        return 0;
    }
    /// Out of BMP?
    if (chr >= 0x10000) {
        /// Get high 10 bits
        hc  = ((chr & 0xFFFF0000) - 0xD8000000) >> 6;
        /// Get low 10 bits
        chr = (chr & 0xFFFF) - 0xDC00;
        /// Wrong surrogate?
        if (hc >= 0x100000 || chr >= 0x400) {
            return 0;
        }
        chr = (hc | chr) + 0x10000;
    }
    *buf++ = (TCHAR)chr;
    return 1;

/// ANSI/OEM output
#    else
    WCHAR wc;

    wc = ff_uni2oem(chr, CODEPAGE);
    /// Is this a DBC?
    if (wc >= 0x100) {
        if (szb < 2) {
            return 0;
        }
        /// Store DBC 1st byte
        *buf++ = (char)(wc >> 8);
        /// Store DBC 2nd byte
        *buf++ = (TCHAR)wc;
        return 2;
    }
    /// Invalid char or buffer overflow?
    if (wc == 0 || szb < 1) {
        return 0;
    }
    /// Store the character
    *buf++ = (TCHAR)wc;
    return 1;
#    endif
}
#endif /* FF_USE_LFN */

#if FF_FS_REENTRANT
/*-----------------------------------------------------------------------*/
/* Request/Release grant to access the volume                            */
/*-----------------------------------------------------------------------*/

/// 1:Ok, 0:timeout
static int lock_volume(
  /// Filesystem object to lock
  FATFS* fs,
  /// System lock required
  int    syslock) {
    int rv;

#    if FF_FS_LOCK
    /// Lock the volume
    rv = ff_mutex_take(fs->ldrv);
    /// System lock reqiered?
    if (rv && syslock) {
        /// Lock the system
        rv = ff_mutex_take(FF_VOLUMES);
        if (rv) {
            /// System lock succeeded
            SysLock = 2;
        }
        else {
            /// Failed system lock
            ff_mutex_give(fs->ldrv);
        }
    }
#    else
    /// Lock the volume (this is to prevent compiler warning)
    rv  = syslock ? ff_mutex_take(fs->ldrv) : ff_mutex_take(fs->ldrv);
#    endif
    return rv;
}

static void unlock_volume(
  /// Filesystem object
  FATFS*  fs,
  /// Result code to be returned
  FRESULT res) {
    if (fs && res != FR_NOT_ENABLED && res != FR_INVALID_DRIVE
        && res != FR_TIMEOUT) {
#    if FF_FS_LOCK
        /// Is the system locked?
        if (SysLock == 2) {
            SysLock = 1;
            ff_mutex_give(FF_VOLUMES);
        }
#    endif
        /// Unlock the volume
        ff_mutex_give(fs->ldrv);
    }
}

#endif

#if FF_FS_LOCK
/*-----------------------------------------------------------------------*/
/* File shareing control functions                                       */
/*-----------------------------------------------------------------------*/

/// Check if the file can be accessed
static FRESULT chk_share(
  /// Directory object pointing the file to be checked
  DIR* dp,
  /// Desired access type (0:Read mode open, 1:Write mode open, 2:Delete or
  /// rename)
  int  acc) {
    UINT i, be;

    /// Search open object table for the object

    be = 0;
    for (i = 0; i < FF_FS_LOCK; i++) {
        /// Existing entry
        if (Files[i].fs) {
            /// Check if the object matches with an open object
            if (Files[i].fs == dp->obj.fs && Files[i].clu == dp->obj.sclust
                && Files[i].ofs == dp->dptr) {
                break;
            }
            /// Blank entry
        }
        else {
            be = 1;
        }
    }
    /// The object has not been opened
    if (i == FF_FS_LOCK) {
        /// Is there a blank entry for new object?
        return (!be && acc != 2) ? FR_TOO_MANY_OPEN_FILES : FR_OK;
    }

    /// The object was opened. Reject any open against writing file and all
    /// write mode open

    return (acc != 0 || Files[i].ctr == 0x100) ? FR_LOCKED : FR_OK;
}

/// Check if an entry is available for a new object
static int enq_share(void) {
    UINT i;

    /// Find a free entry
    for (i = 0; i < FF_FS_LOCK && Files[i].fs; i++)
        ;
    return (i == FF_FS_LOCK) ? 0 : 1;
}

/// Increment object open counter and returns its index (0:Internal error)
static UINT inc_share(
  /// Directory object pointing the file to register or increment
  DIR* dp,
  /// Desired access (0:Read, 1:Write, 2:Delete/Rename)
  int  acc) {
    UINT i;

    /// Find the object
    for (i = 0; i < FF_FS_LOCK; i++) {
        if (Files[i].fs == dp->obj.fs && Files[i].clu == dp->obj.sclust
            && Files[i].ofs == dp->dptr) {
            break;
        }
    }

    /// Not opened. Register it as new.
    if (i == FF_FS_LOCK) {
        /// Find a free entry
        for (i = 0; i < FF_FS_LOCK && Files[i].fs; i++)
            ;
        /// No free entry to register (int err)
        if (i == FF_FS_LOCK) {
            return 0;
        }
        Files[i].fs  = dp->obj.fs;
        Files[i].clu = dp->obj.sclust;
        Files[i].ofs = dp->dptr;
        Files[i].ctr = 0;
    }

    /// Access violation (int err)
    if (acc >= 1 && Files[i].ctr) {
        return 0;
    }

    /// Set semaphore value
    Files[i].ctr = acc ? 0x100 : Files[i].ctr + 1;

    /// Index number origin from 1
    return i + 1;
}

/// Decrement object open counter
static FRESULT dec_share(
  /// Semaphore index (1..)
  UINT i) {
    UINT    n;
    FRESULT res;

    /// Index number origin from 0
    if (--i < FF_FS_LOCK) {
        n = Files[i].ctr;
        /// If write mode open, delete the object semaphore
        if (n == 0x100) {
            n = 0;
        }
        /// Decrement read mode open count
        if (n > 0) {
            n--;
        }
        Files[i].ctr = n;
        /// Delete the object semaphore if open count becomes zero
        if (n == 0) {
            /// Free the entry <<<If this memory write operation is not in
            /// atomic, FF_FS_REENTRANT == 1 and FF_VOLUMES > 1, there is a
            /// potential error in this process >>>
            Files[i].fs = 0;
        }
        res = FR_OK;
    }
    else {
        /// Invalid index number
        res = FR_INT_ERR;
    }
    return res;
}

/// Clear all lock entries of the volume
static void clear_share(FATFS* fs) {
    UINT i;

    for (i = 0; i < FF_FS_LOCK; i++) {
        if (Files[i].fs == fs) {
            Files[i].fs = 0;
        }
    }
}

#endif /* FF_FS_LOCK */

/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the filesystem object                */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY
/// Returns FR_OK or FR_DISK_ERR
static FRESULT sync_window(
  /// Filesystem object
  FATFS* fs) {
    FRESULT res = FR_OK;

    /// Is the disk access window dirty?
    if (fs->wflag) {
        /// Write it back into the volume
        if (disk_write(fs->pdrv, fs->win, fs->winsect, 1) == RES_OK) {
            /// Clear window dirty flag
            fs->wflag = 0;
            /// Is it in the 1st FAT?
            if (fs->winsect - fs->fatbase < fs->fsize) {
                /// Reflect it to 2nd FAT if needed
                if (fs->n_fats == 2) {
                    disk_write(fs->pdrv, fs->win, fs->winsect + fs->fsize, 1);
                }
            }
        }
        else {
            res = FR_DISK_ERR;
        }
    }
    return res;
}
#endif

/// Returns FR_OK or FR_DISK_ERR
static FRESULT move_window(
  /// Filesystem object
  FATFS* fs,
  /// Sector LBA to make appearance in the fs->win[]
  LBA_t  sect) {
    FRESULT res = FR_OK;

    /// Window offset changed?
    if (sect != fs->winsect) {
#if !FF_FS_READONLY
        /// Flush the window
        res = sync_window(fs);
#endif
        /// Fill sector window with new data
        if (res == FR_OK) {
            if (disk_read(fs->pdrv, fs->win, sect, 1) != RES_OK) {
                /// Invalidate window if read data is not valid
                sect = (LBA_t)0 - 1;
                res  = FR_DISK_ERR;
            }
            fs->winsect = sect;
        }
    }
    return res;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Synchronize filesystem and data on the storage                        */
/*-----------------------------------------------------------------------*/

/// Returns FR_OK or FR_DISK_ERR
static FRESULT sync_fs(
  /// Filesystem object
  FATFS* fs) {
    FRESULT res;

    res = sync_window(fs);
    if (res == FR_OK) {
        /// FAT32: Update FSInfo sector if needed
        if (fs->fs_type == FS_FAT32 && fs->fsi_flag == 1) {
            /// Create FSInfo structure

            memset(fs->win, 0, sizeof fs->win);
            /// Boot signature
            st_word(fs->win + BS_55AA, 0xAA55);
            /// Leading signature
            st_dword(fs->win + FSI_LeadSig, 0x41615252);
            /// Structure signature
            st_dword(fs->win + FSI_StrucSig, 0x61417272);
            /// Number of free clusters
            st_dword(fs->win + FSI_Free_Count, fs->free_clst);
            /// Last allocated culuster
            st_dword(fs->win + FSI_Nxt_Free, fs->last_clst);
            /// Write it into the FSInfo sector (Next to VBR)
            fs->winsect = fs->volbase + 1;
            disk_write(fs->pdrv, fs->win, fs->winsect, 1);
            fs->fsi_flag = 0;
        }
        /// Make sure that no pending write process in the lower layer

        if (disk_ioctl(fs->pdrv, CTRL_SYNC, 0) != RES_OK) {
            res = FR_DISK_ERR;
        }
    }

    return res;
}

#endif

/*-----------------------------------------------------------------------*/
/* Get physical sector number from cluster number                        */
/*-----------------------------------------------------------------------*/

/// !=0:Sector number, 0:Failed (invalid cluster#)
static LBA_t clst2sect(
  /// Filesystem object
  FATFS* fs,
  /// Cluster# to be converted
  DWORD  clst) {
    /// Cluster number is origin from 2
    clst -= 2;
    /// Is it invalid cluster number?
    if (clst >= fs->n_fatent - 2) {
        return 0;
    }
    /// Start sector number of the cluster
    return fs->database + (LBA_t)fs->csize * clst;
}

/*-----------------------------------------------------------------------*/
/* FAT access - Read value of an FAT entry                               */
/*-----------------------------------------------------------------------*/

/// 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status
static DWORD get_fat(
  // Corresponding object
  FFOBJID* obj,
  // Cluster number to get the value
  DWORD    clst) {
    UINT   wc, bc;
    DWORD  val;
    FATFS* fs = obj->fs;

    // Check if in valid range
    if (clst < 2 || clst >= fs->n_fatent) {
        // Internal error
        val = 1;
    }
    else {
        // Default value falls on disk error
        val = 0xFFFFFFFF;

        switch (fs->fs_type) {
            case FS_FAT12:
                bc  = (UINT)clst;
                bc += bc / 2;
                if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK) {
                    break;
                }
                // Get 1st byte of the entry
                wc = fs->win[bc++ % SS(fs)];
                if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK) {
                    break;
                }
                // Merge 2nd byte of the entry
                wc  |= fs->win[bc % SS(fs)] << 8;
                // Adjust bit position
                val  = (clst & 1) ? (wc >> 4) : (wc & 0xFFF);
                break;

            case FS_FAT16:
                if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)))
                    != FR_OK) {
                    break;
                }
                // Simple WORD array
                val = ld_word(fs->win + clst * 2 % SS(fs));
                break;

            case FS_FAT32:
                if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)))
                    != FR_OK) {
                    break;
                }
                // Simple DWORD array but mask out upper 4 bits
                val = ld_dword(fs->win + clst * 4 % SS(fs)) & 0x0FFFFFFF;
                break;
#if FF_FS_EXFAT
            case FS_EXFAT:
                // Object except root dir must have valid data length
                if ((obj->objsize != 0 && obj->sclust != 0) || obj->stat == 0) {
                    // Offset from start cluster
                    DWORD cofs = clst - obj->sclust;
                    // Number of clusters - 1
                    DWORD clen = (DWORD)((LBA_t)((obj->objsize - 1) / SS(fs))
                                         / fs->csize);

                    // Is it a contiguous chain?
                    if (obj->stat == 2 && cofs <= clen) {
                        // No data on the FAT, generate the value
                        val = (cofs == clen) ? 0x7FFFFFFF : clst + 1;
                        break;
                    }
                    // Is it in the 1st fragment?
                    if (obj->stat == 3 && cofs < obj->n_cont) {
                        // Generate the value
                        val = clst + 1;
                        break;
                    }
                    // Get value from FAT if FAT chain is valid
                    if (obj->stat != 2) {
                        // Is it on the growing edge?
                        if (obj->n_frag != 0) {
                            // Generate EOC
                            val = 0x7FFFFFFF;
                        }
                        else {
                            if (move_window(fs,
                                            fs->fatbase + (clst / (SS(fs) / 4)))
                                != FR_OK) {
                                break;
                            }
                            val = ld_dword(fs->win + clst * 4 % SS(fs))
                                & 0x7FFFFFFF;
                        }
                        break;
                    }
                }
                // Internal error
                val = 1;
                break;
#endif
            default:
                // Internal error
                val = 1;
        }
    }

    return val;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT access - Change value of an FAT entry                             */
/*-----------------------------------------------------------------------*/

static FRESULT put_fat(FATFS* fs,
                       // FR_OK(0):succeeded, !=0:error
                       DWORD  clst,
                       // Corresponding filesystem object
                       DWORD  val
                       // FAT index number (cluster number) to be changed
                       )
// New value to be set to the entry
{
    UINT    bc;
    BYTE*   p;
    FRESULT res = FR_INT_ERR;

    if (clst >= 2 && clst < fs->n_fatent) {
        switch (fs->fs_type) {
                // Check if in valid range
            case FS_FAT12:
                bc   = (UINT)clst;
                bc  += bc / 2;
                res  = move_window(fs, fs->fatbase + (bc / SS(fs)));
                // bc: byte offset of the entry
                if (res != FR_OK) {
                    break;
                }
                p  = fs->win + bc++ % SS(fs);
                *p = (clst & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;
                fs->wflag = 1;
                // Update 1st byte
                res       = move_window(fs, fs->fatbase + (bc / SS(fs)));
                if (res != FR_OK) {
                    break;
                }
                p         = fs->win + bc % SS(fs);
                *p        = (clst & 1) ? (BYTE)(val >> 4)
                                       : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));
                fs->wflag = 1;
                // Update 2nd byte
                break;

            case FS_FAT16:
                res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
                if (res != FR_OK) {
                    break;
                }
                st_word(fs->win + clst * 2 % SS(fs), (WORD)val);
                fs->wflag = 1;
                // Simple WORD array
                break;

            case FS_FAT32:
#    if FF_FS_EXFAT
            case FS_EXFAT:
#    endif
                res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)));
                if (res != FR_OK) {
                    break;
                }
                if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
                    val = (val & 0x0FFFFFFF)
                        | (ld_dword(fs->win + clst * 4 % SS(fs)) & 0xF0000000);
                }
                st_dword(fs->win + clst * 4 % SS(fs), val);
                fs->wflag = 1;
                break;
        }
    }
    return res;
}

#endif /* !FF_FS_READONLY */

#if FF_FS_EXFAT && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* exFAT: Accessing FAT and Allocation Bitmap                            */
/*-----------------------------------------------------------------------*/

/*--------------------------------------*/
/* Find a contiguous free cluster block */
/*--------------------------------------*/

// 0:Not found, 2..:Cluster block found, 0xFFFFFFFF:Disk error
static DWORD find_bitmap(
  // Filesystem object
  FATFS* fs,
  // Cluster number to scan from
  DWORD  clst,
  // Number of contiguous clusters to find (1..)
  DWORD  ncl) {
    BYTE  bm, bv;
    UINT  i;
    DWORD val, scl, ctr;

    // The first bit in the bitmap corresponds to cluster #2
    clst -= 2;
    if (clst >= fs->n_fatent - 2) {
        clst = 0;
    }
    scl = val = clst;
    ctr       = 0;
    for (;;) {
        if (move_window(fs, fs->bitbase + val / 8 / SS(fs)) != FR_OK) {
            return 0xFFFFFFFF;
        }
        i  = val / 8 % SS(fs);
        bm = 1 << (val % 8);
        do {
            do {
                // Get bit value
                bv   = fs->win[i] & bm;
                bm <<= 1;
                // Next cluster (with wrap-around)
                if (++val >= fs->n_fatent - 2) {
                    val = 0;
                    bm  = 0;
                    i   = SS(fs);
                }
                // Is it a free cluster?
                if (bv == 0) {
                    // Check if run length is sufficient for required
                    if (++ctr == ncl) {
                        return scl + 2;
                    }
                }
                else {
                    // Encountered a cluster in-use, restart to scan
                    scl = val;
                    ctr = 0;
                }
                // All cluster scanned?
                if (val == clst) {
                    return 0;
                }
            } while (bm != 0);
            bm = 1;
        } while (++i < SS(fs));
    }
}

/*----------------------------------------*/
/* Set/Clear a block of allocation bitmap */
/*----------------------------------------*/

static FRESULT change_bitmap(
  // Filesystem object
  FATFS* fs,
  // Cluster number to change from
  DWORD  clst,
  // Number of clusters to be changed
  DWORD  ncl,
  // bit value to be set (0 or 1)
  int    bv) {
    BYTE  bm;
    UINT  i;
    LBA_t sect;

    // The first bit corresponds to cluster #2
    clst -= 2;
    // Sector address
    sect  = fs->bitbase + clst / 8 / SS(fs);
    // Byte offset in the sector
    i     = clst / 8 % SS(fs);
    // Bit mask in the byte
    bm    = 1 << (clst % 8);
    for (;;) {
        if (move_window(fs, sect++) != FR_OK) {
            return FR_DISK_ERR;
        }
        do {
            do {
                // Is the bit expected value?
                if (bv == (int)((fs->win[i] & bm) != 0)) {
                    return FR_INT_ERR;
                }
                // Flip the bit
                fs->win[i] ^= bm;
                fs->wflag   = 1;
                // All bits processed?
                if (--ncl == 0) {
                    return FR_OK;
                }
                // Next bit
            } while (bm <<= 1);
            bm = 1;
            // Next byte
        } while (++i < SS(fs));
        i = 0;
    }
}

/*---------------------------------------------*/
/* Fill the first fragment of the FAT chain    */
/*---------------------------------------------*/

static FRESULT fill_first_frag(
  // Pointer to the corresponding object
  FFOBJID* obj) {
    FRESULT res;
    DWORD   cl, n;

    // Has the object been changed 'fragmented' in this session?
    if (obj->stat == 3) {
        // Create cluster chain on the FAT
        for (cl = obj->sclust, n = obj->n_cont; n; cl++, n--) {
            res = put_fat(obj->fs, cl, cl + 1);
            if (res != FR_OK) {
                return res;
            }
        }
        // Change status 'FAT chain is valid'
        obj->stat = 0;
    }
    return FR_OK;
}

/*---------------------------------------------*/
/* Fill the last fragment of the FAT chain     */
/*---------------------------------------------*/

static FRESULT fill_last_frag(
  // Pointer to the corresponding object
  FFOBJID* obj,
  // Last cluster of the fragment
  DWORD    lcl,
  // Value to set the last FAT entry
  DWORD    term) {
    FRESULT res;

    // Create the chain of last fragment
    while (obj->n_frag > 0) {
        res = put_fat(obj->fs, lcl - obj->n_frag + 1,
                      (obj->n_frag > 1) ? lcl - obj->n_frag + 2 : term);
        if (res != FR_OK) {
            return res;
        }
        obj->n_frag--;
    }
    return FR_OK;
}

#endif /* FF_FS_EXFAT && !FF_FS_READONLY */

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT handling - Remove a cluster chain                                 */
/*-----------------------------------------------------------------------*/

// FR_OK(0):succeeded, !=0:error
static FRESULT remove_chain(
  // Corresponding object
  FFOBJID* obj,
  // Cluster to remove a chain from
  DWORD    clst,
  // Previous cluster of clst (0 if entire chain)
  DWORD    pclst) {
    FRESULT res = FR_OK;
    DWORD   nxt;
    FATFS*  fs = obj->fs;
#    if FF_FS_EXFAT || FF_USE_TRIM
    DWORD scl = clst, ecl = clst;
#    endif
#    if FF_USE_TRIM
    LBA_t rt[2];
#    endif

    // Check if in valid range
    if (clst < 2 || clst >= fs->n_fatent) {
        return FR_INT_ERR;
    }

    // Mark the previous cluster 'EOC' on the FAT if it exists

    if (pclst != 0
        && (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT || obj->stat != 2)) {
        res = put_fat(fs, pclst, 0xFFFFFFFF);
        if (res != FR_OK) {
            return res;
        }
    }

    // Remove the chain

    do {
        // Get cluster status
        nxt = get_fat(obj, clst);
        // Empty cluster?
        if (nxt == 0) {
            break;
        }
        // Internal error?
        if (nxt == 1) {
            return FR_INT_ERR;
        }
        // Disk error?
        if (nxt == 0xFFFFFFFF) {
            return FR_DISK_ERR;
        }
        if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
            // Mark the cluster 'free' on the FAT
            res = put_fat(fs, clst, 0);
            if (res != FR_OK) {
                return res;
            }
        }
        // Update FSINFO
        if (fs->free_clst < fs->n_fatent - 2) {
            fs->free_clst++;
            fs->fsi_flag |= 1;
        }
#    if FF_FS_EXFAT || FF_USE_TRIM
        // Is next cluster contiguous?
        if (ecl + 1 == nxt) {
            ecl = nxt;
            // End of contiguous cluster block
        }
        else {
#        if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                // Mark the cluster block 'free' on the bitmap
                res = change_bitmap(fs, scl, ecl - scl + 1, 0);
                if (res != FR_OK) {
                    return res;
                }
            }
#        endif
#        if FF_USE_TRIM
            // Start of data area to be freed
            rt[0] = clst2sect(fs, scl);
            // End of data area to be freed
            rt[1] = clst2sect(fs, ecl) + fs->csize - 1;
            // Inform storage device that the data in the block may be erased
            disk_ioctl(fs->pdrv, CTRL_TRIM, rt);
#        endif
            scl = ecl = nxt;
        }
#    endif
        // Next cluster
        clst = nxt;
        // Repeat while not the last link
    } while (clst < fs->n_fatent);

#    if FF_FS_EXFAT
    // Some post processes for chain status

    if (fs->fs_type == FS_EXFAT) {
        // Has the entire chain been removed?
        if (pclst == 0) {
            // Change the chain status 'initial'
            obj->stat = 0;
        }
        else {
            // Is it a fragmented chain from the beginning of this session?
            if (obj->stat == 0) {
                // Follow the chain to check if it gets contiguous
                clst = obj->sclust;
                while (clst != pclst) {
                    nxt = get_fat(obj, clst);
                    if (nxt < 2) {
                        return FR_INT_ERR;
                    }
                    if (nxt == 0xFFFFFFFF) {
                        return FR_DISK_ERR;
                    }
                    // Not contiguous?
                    if (nxt != clst + 1) {
                        break;
                    }
                    clst++;
                }
                // Has the chain got contiguous again?
                if (clst == pclst) {
                    // Change the chain status 'contiguous'
                    obj->stat = 2;
                }
            }
            else {
                // Was the chain fragmented in this session and got contiguous
                // again?
                if (obj->stat == 3 && pclst >= obj->sclust
                    && pclst <= obj->sclust + obj->n_cont) {
                    // Change the chain status 'contiguous'
                    obj->stat = 2;
                }
            }
        }
    }
#    endif
    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* FAT handling - Stretch a chain or Create a new chain                  */
/*-----------------------------------------------------------------------*/

// 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster#
static DWORD create_chain(
  // Corresponding object
  FFOBJID* obj,
  // Cluster# to stretch, 0:Create a new chain
  DWORD    clst) {
    DWORD   cs, ncl, scl;
    FRESULT res;
    FATFS*  fs = obj->fs;

    // Create a new chain
    if (clst == 0) {
        // Suggested cluster to start to find
        scl = fs->last_clst;
        if (scl == 0 || scl >= fs->n_fatent) {
            scl = 1;
        }
    }
    // Stretch a chain
    else {
        // Check the cluster status
        cs = get_fat(obj, clst);
        // Test for insanity
        if (cs < 2) {
            return 1;
        }
        // Test for disk error
        if (cs == 0xFFFFFFFF) {
            return cs;
        }
        // It is already followed by next cluster
        if (cs < fs->n_fatent) {
            return cs;
        }
        // Cluster to start to find
        scl = clst;
    }
    // No free cluster
    if (fs->free_clst == 0) {
        return 0;
    }

#    if FF_FS_EXFAT
    // On the exFAT volume
    if (fs->fs_type == FS_EXFAT) {
        // Find a free cluster
        ncl = find_bitmap(fs, scl, 1);
        // No free cluster or hard error?
        if (ncl == 0 || ncl == 0xFFFFFFFF) {
            return ncl;
        }
        // Mark the cluster 'in use'
        res = change_bitmap(fs, ncl, 1, 1);
        if (res == FR_INT_ERR) {
            return 1;
        }
        if (res == FR_DISK_ERR) {
            return 0xFFFFFFFF;
        }
        // Is it a new chain?
        if (clst == 0) {
            // Set status 'contiguous'
            obj->stat = 2;
            // It is a stretched chain
        }
        else {
            // Is the chain got fragmented?
            if (obj->stat == 2 && ncl != scl + 1) {
                // Set size of the contiguous part
                obj->n_cont = scl - obj->sclust;
                // Change status 'just fragmented'
                obj->stat   = 3;
            }
        }
        // Is the file non-contiguous?
        if (obj->stat != 2) {
            // Is the cluster next to previous one?
            if (ncl == clst + 1) {
                // Increment size of last framgent
                obj->n_frag = obj->n_frag ? obj->n_frag + 1 : 2;
                // New fragment
            }
            else {
                if (obj->n_frag == 0) {
                    obj->n_frag = 1;
                }
                // Fill last fragment on the FAT and link it to new one
                res = fill_last_frag(obj, clst, ncl);
                if (res == FR_OK) {
                    obj->n_frag = 1;
                }
            }
        }
    }
    else
#    endif
    // On the FAT/FAT32 volume
    {
        ncl = 0;
        // Stretching an existing chain?
        if (scl == clst) {
            // Test if next cluster is free
            ncl = scl + 1;
            if (ncl >= fs->n_fatent) {
                ncl = 2;
            }
            // Get next cluster status
            cs = get_fat(obj, ncl);
            // Test for error
            if (cs == 1 || cs == 0xFFFFFFFF) {
                return cs;
            }
            // Not free?
            if (cs != 0) {
                // Start at suggested cluster if it is valid
                cs = fs->last_clst;
                if (cs >= 2 && cs < fs->n_fatent) {
                    scl = cs;
                }
                ncl = 0;
            }
        }
        // The new cluster cannot be contiguous and find another fragment
        if (ncl == 0) {
            // Start cluster
            ncl = scl;
            for (;;) {
                // Next cluster
                ncl++;
                // Check wrap-around
                if (ncl >= fs->n_fatent) {
                    ncl = 2;
                    // No free cluster found?
                    if (ncl > scl) {
                        return 0;
                    }
                }
                // Get the cluster status
                cs = get_fat(obj, ncl);
                // Found a free cluster?
                if (cs == 0) {
                    break;
                }
                // Test for error
                if (cs == 1 || cs == 0xFFFFFFFF) {
                    return cs;
                }
                // No free cluster found?
                if (ncl == scl) {
                    return 0;
                }
            }
        }
        // Mark the new cluster 'EOC'
        res = put_fat(fs, ncl, 0xFFFFFFFF);
        if (res == FR_OK && clst != 0) {
            // Link it from the previous one if needed
            res = put_fat(fs, clst, ncl);
        }
    }

    // Update FSINFO if function succeeded.
    if (res == FR_OK) {
        fs->last_clst = ncl;
        if (fs->free_clst <= fs->n_fatent - 2) {
            fs->free_clst--;
        }
        fs->fsi_flag |= 1;
    }
    else {
        // Failed. Generate error status
        ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;
    }

    // Return new cluster number or error status
    return ncl;
}

#endif /* !FF_FS_READONLY */

#if FF_USE_FASTSEEK
/*-----------------------------------------------------------------------*/
/* FAT handling - Convert offset into cluster with link map table        */
/*-----------------------------------------------------------------------*/

// <2:Error, >=2:Cluster number
static DWORD clmt_clust(
  // Pointer to the file object
  FIL*    fp,
  // File offset to be converted to cluster#
  FSIZE_t ofs) {
    DWORD  cl, ncl;
    DWORD* tbl;
    FATFS* fs = fp->obj.fs;

    // Top of CLMT
    tbl       = fp->cltbl + 1;
    // Cluster order from top of the file
    cl        = (DWORD)(ofs / SS(fs) / fs->csize);
    for (;;) {
        // Number of cluters in the fragment
        ncl = *tbl++;
        // End of table? (error)
        if (ncl == 0) {
            return 0;
        }
        // In this fragment?
        if (cl < ncl) {
            break;
        }
        // Next fragment
        cl -= ncl;
        tbl++;
    }
    // Return the cluster number
    return cl + *tbl;
}

#endif /* FF_USE_FASTSEEK */

/*-----------------------------------------------------------------------*/
/* Directory handling - Fill a cluster with zeros                        */
/*-----------------------------------------------------------------------*/

#if !FF_FS_READONLY
// Returns FR_OK or FR_DISK_ERR
static FRESULT dir_clear(
  // Filesystem object
  FATFS* fs,
  // Directory table to clear
  DWORD  clst) {
    LBA_t sect;
    UINT  n, szb;
    BYTE* ibuf;

    // Flush disk access window
    if (sync_window(fs) != FR_OK) {
        return FR_DISK_ERR;
    }
    // Top of the cluster
    sect        = clst2sect(fs, clst);
    // Set window to top of the cluster
    fs->winsect = sect;
    // Clear window buffer
    memset(fs->win, 0, sizeof fs->win);
    // Quick table clear by using multi-secter write
#    if FF_USE_LFN == 3
    // Allocate a temporary buffer

    for (szb = ((DWORD)fs->csize * SS(fs) >= MAX_MALLOC) ? MAX_MALLOC
                                                         : fs->csize * SS(fs),
        ibuf = 0;
         szb > SS(fs) && (ibuf = ff_memalloc(szb)) == 0; szb /= 2)
        ;
    // Buffer allocated?
    if (szb > SS(fs)) {
        memset(ibuf, 0, szb);
        // Bytes -> Sectors
        szb /= SS(fs);
        // Fill the cluster with 0
        for (n = 0; n < fs->csize
                    && disk_write(fs->pdrv, ibuf, sect + n, szb) == RES_OK;
             n += szb)
            ;
        ff_memfree(ibuf);
    }
    else
#    endif
    {
        // Use window buffer (many single-sector writes may take a time)
        ibuf = fs->win;
        szb  = 1;
        // Fill the cluster with 0
        for (n = 0; n < fs->csize
                    && disk_write(fs->pdrv, ibuf, sect + n, szb) == RES_OK;
             n += szb)
            ;
    }
    return (n == fs->csize) ? FR_OK : FR_DISK_ERR;
}
#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Directory handling - Set directory index                              */
/*-----------------------------------------------------------------------*/

// FR_OK(0):succeeded, !=0:error
static FRESULT dir_sdi(
  // Pointer to directory object
  DIR*  dp,
  // Offset of directory table
  DWORD ofs) {
    DWORD  csz, clst;
    FATFS* fs = dp->obj.fs;

    // Check range of offset and alignment
    if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX
                                                                : MAX_DIR)
        || ofs % SZDIRE) {
        return FR_INT_ERR;
    }
    // Set current offset
    dp->dptr = ofs;
    // Table start cluster (0:root)
    clst     = dp->obj.sclust;
    // Replace cluster# 0 with root cluster#
    if (clst == 0 && fs->fs_type >= FS_FAT32) {
        clst = (DWORD)fs->dirbase;
        // exFAT: Root dir has an FAT chain
        if (FF_FS_EXFAT) {
            dp->obj.stat = 0;
        }
    }

    // Static table (root-directory on the FAT volume)
    if (clst == 0) {
        // Is index out of range?
        if (ofs / SZDIRE >= fs->n_rootdir) {
            return FR_INT_ERR;
        }
        dp->sect = fs->dirbase;

        // Dynamic table (sub-directory or root-directory on the FAT32/exFAT
        // volume)
    }
    else {
        // Bytes per cluster
        csz = (DWORD)fs->csize * SS(fs);
        // Follow cluster chain
        while (ofs >= csz) {
            // Get next cluster
            clst = get_fat(&dp->obj, clst);
            // Disk error
            if (clst == 0xFFFFFFFF) {
                return FR_DISK_ERR;
            }
            // Reached to end of table or internal error
            if (clst < 2 || clst >= fs->n_fatent) {
                return FR_INT_ERR;
            }
            ofs -= csz;
        }
        dp->sect = clst2sect(fs, clst);
    }
    // Current cluster#
    dp->clust = clst;
    if (dp->sect == 0) {
        return FR_INT_ERR;
    }
    // Sector# of the directory entry
    dp->sect += ofs / SS(fs);
    // Pointer to the entry in the win[]
    dp->dir   = fs->win + (ofs % SS(fs));

    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory table index next                  */
/*-----------------------------------------------------------------------*/

// FR_OK(0):succeeded, FR_NO_FILE:End of table, FR_DENIED:Could not stretch
static FRESULT dir_next(
  // Pointer to the directory object
  DIR* dp,
  // 0: Do not stretch table, 1: Stretch table if needed
  int  stretch) {
    DWORD  ofs, clst;
    FATFS* fs = dp->obj.fs;

    // Next entry
    ofs       = dp->dptr + SZDIRE;
    // Disable it if the offset reached the max value
    if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? MAX_DIR_EX
                                                                : MAX_DIR)) {
        dp->sect = 0;
    }
    // Report EOT if it has been disabled
    if (dp->sect == 0) {
        return FR_NO_FILE;
    }

    // Sector changed?
    if (ofs % SS(fs) == 0) {
        // Next sector
        dp->sect++;

        // Static table
        if (dp->clust == 0) {
            // Report EOT if it reached end of static table
            if (ofs / SZDIRE >= fs->n_rootdir) {
                dp->sect = 0;
                return FR_NO_FILE;
            }
        }
        // Dynamic table
        else {
            // Cluster changed?
            if ((ofs / SS(fs) & (fs->csize - 1)) == 0) {
                // Get next cluster
                clst = get_fat(&dp->obj, dp->clust);
                // Internal error
                if (clst <= 1) {
                    return FR_INT_ERR;
                }
                // Disk error
                if (clst == 0xFFFFFFFF) {
                    return FR_DISK_ERR;
                }
                // It reached end of dynamic table
                if (clst >= fs->n_fatent) {
#if !FF_FS_READONLY
                    // If no stretch, report EOT
                    if (!stretch) {
                        dp->sect = 0;
                        return FR_NO_FILE;
                    }
                    // Allocate a cluster
                    clst = create_chain(&dp->obj, dp->clust);
                    // No free cluster
                    if (clst == 0) {
                        return FR_DENIED;
                    }
                    // Internal error
                    if (clst == 1) {
                        return FR_INT_ERR;
                    }
                    // Disk error
                    if (clst == 0xFFFFFFFF) {
                        return FR_DISK_ERR;
                    }
                    // Clean up the stretched table
                    if (dir_clear(fs, clst) != FR_OK) {
                        return FR_DISK_ERR;
                    }
                    // exFAT: The directory has been stretched
                    if (FF_FS_EXFAT) {
                        dp->obj.stat |= 4;
                    }
#else
                    // (this line is to suppress compiler warning)
                    if (!stretch) {
                        dp->sect = 0;
                    }
                    // Report EOT
                    dp->sect = 0;
                    return FR_NO_FILE;
#endif
                }
                // Initialize data for new cluster
                dp->clust = clst;
                dp->sect  = clst2sect(fs, clst);
            }
        }
    }
    // Current entry
    dp->dptr = ofs;
    // Pointer to the entry in the win[]
    dp->dir  = fs->win + ofs % SS(fs);

    return FR_OK;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Directory handling - Reserve a block of directory entries             */
/*-----------------------------------------------------------------------*/

// FR_OK(0):succeeded, !=0:error
static FRESULT dir_alloc(
  // Pointer to the directory object
  DIR* dp,
  // Number of contiguous entries to allocate
  UINT n_ent) {
    FRESULT res;
    UINT    n;
    FATFS*  fs = dp->obj.fs;

    res        = dir_sdi(dp, 0);
    if (res == FR_OK) {
        n = 0;
        do {
            res = move_window(fs, dp->sect);
            if (res != FR_OK) {
                break;
            }
#    if FF_FS_EXFAT
            // Is the entry free?
            if ((fs->fs_type == FS_EXFAT)
                  ? (int)((dp->dir[XDIR_Type] & 0x80) == 0)
                  : (int)(dp->dir[DIR_Name] == DDEM
                          || dp->dir[DIR_Name] == 0)) {
#    else
            // Is the entry free?
            if (dp->dir[DIR_Name] == DDEM || dp->dir[DIR_Name] == 0) {
#    endif
                // Is a block of contiguous free entries found?
                if (++n == n_ent) {
                    break;
                }
            }
            else {
                // Not a free entry, restart to search
                n = 0;
            }
            // Next entry with table stretch enabled
            res = dir_next(dp, 1);
        } while (res == FR_OK);
    }

    // No directory entry to allocate
    if (res == FR_NO_FILE) {
        res = FR_DENIED;
    }
    return res;
}

#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* FAT: Directory handling - Load/Store start cluster number             */
/*-----------------------------------------------------------------------*/

// Returns the top cluster value of the SFN entry
static DWORD ld_clust(
  // Pointer to the fs object
  FATFS*      fs,
  // Pointer to the key entry
  const BYTE* dir) {
    DWORD cl;

    cl = ld_word(dir + DIR_FstClusLO);
    if (fs->fs_type == FS_FAT32) {
        cl |= (DWORD)ld_word(dir + DIR_FstClusHI) << 16;
    }

    return cl;
}

#if !FF_FS_READONLY
static void st_clust(
  // Pointer to the fs object
  FATFS* fs,
  // Pointer to the key entry
  BYTE*  dir,
  // Value to be set
  DWORD  cl) {
    st_word(dir + DIR_FstClusLO, (WORD)cl);
    if (fs->fs_type == FS_FAT32) {
        st_word(dir + DIR_FstClusHI, (WORD)(cl >> 16));
    }
}
#endif

#if FF_USE_LFN
/*--------------------------------------------------------*/
/* FAT-LFN: Compare a part of file name with an LFN entry */
/*--------------------------------------------------------*/

// 1:matched, 0:not matched
static int cmp_lfn(
  // Pointer to the LFN working buffer to be compared
  const WCHAR* lfnbuf,
  // Pointer to the directory entry containing the part of LFN
  BYTE*        dir) {
    UINT  i, s;
    WCHAR wc, uc;

    // Check LDIR_FstClusLO
    if (ld_word(dir + LDIR_FstClusLO) != 0) {
        return 0;
    }

    // Offset in the LFN buffer
    i = ((dir[LDIR_Ord] & 0x3F) - 1) * 13;

    // Process all characters in the entry
    for (wc = 1, s = 0; s < 13; s++) {
        // Pick an LFN character
        uc = ld_word(dir + LfnOfs[s]);
        if (wc != 0) {
            // Compare it
            if (i >= FF_MAX_LFN + 1
                || ff_wtoupper(uc) != ff_wtoupper(lfnbuf[i++])) {
                // Not matched
                return 0;
            }
            wc = uc;
        }
        else {
            // Check filler
            if (uc != 0xFFFF) {
                return 0;
            }
        }
    }

    // Last segment matched but different length
    if ((dir[LDIR_Ord] & LLEF) && wc && lfnbuf[i]) {
        return 0;
    }

    // The part of LFN matched
    return 1;
}

#    if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT
/*-----------------------------------------------------*/
/* FAT-LFN: Pick a part of file name from an LFN entry */
/*-----------------------------------------------------*/

// 1:succeeded, 0:buffer overflow or invalid LFN entry
static int pick_lfn(
  // Pointer to the LFN working buffer
  WCHAR* lfnbuf,
  // Pointer to the LFN entry
  BYTE*  dir) {
    UINT  i, s;
    WCHAR wc, uc;

    // Check LDIR_FstClusLO is 0
    if (ld_word(dir + LDIR_FstClusLO) != 0) {
        return 0;
    }

    // Offset in the LFN buffer
    i = ((dir[LDIR_Ord] & ~LLEF) - 1) * 13;

    // Process all characters in the entry
    for (wc = 1, s = 0; s < 13; s++) {
        // Pick an LFN character
        uc = ld_word(dir + LfnOfs[s]);
        if (wc != 0) {
            // Buffer overflow?
            if (i >= FF_MAX_LFN + 1) {
                return 0;
            }
            // Store it
            lfnbuf[i++] = wc = uc;
        }
        else {
            // Check filler
            if (uc != 0xFFFF) {
                return 0;
            }
        }
    }

    // Put terminator if it is the last LFN part and not terminated
    if (dir[LDIR_Ord] & LLEF && wc != 0) {
        // Buffer overflow?
        if (i >= FF_MAX_LFN + 1) {
            return 0;
        }
        lfnbuf[i] = 0;
    }

    // The part of LFN is valid
    return 1;
}
#    endif

#    if !FF_FS_READONLY
/*-----------------------------------------*/
/* FAT-LFN: Create an entry of LFN entries */
/*-----------------------------------------*/

static void put_lfn(
  // Pointer to the LFN
  const WCHAR* lfn,
  // Pointer to the LFN entry to be created
  BYTE*        dir,
  // LFN order (1-20)
  BYTE         ord,
  // Checksum of the corresponding SFN
  BYTE         sum) {
    UINT  i, s;
    WCHAR wc;

    // Set checksum
    dir[LDIR_Chksum] = sum;
    // Set attribute. LFN entry
    dir[LDIR_Attr]   = AM_LFN;
    dir[LDIR_Type]   = 0;
    st_word(dir + LDIR_FstClusLO, 0);

    // Get offset in the LFN working buffer
    i = (ord - 1) * 13;
    s = wc = 0;
    do {
        // Get an effective character
        if (wc != 0xFFFF) {
            wc = lfn[i++];
        }
        // Put it
        st_word(dir + LfnOfs[s], wc);
        // Padding characters for following items
        if (wc == 0) {
            wc = 0xFFFF;
        }
    } while (++s < 13);
    // Last LFN part is the start of LFN sequence
    if (wc == 0xFFFF || !lfn[i]) {
        ord |= LLEF;
    }
    // Set the LFN order
    dir[LDIR_Ord] = ord;
}

#    endif /* !FF_FS_READONLY */
#endif     /* FF_USE_LFN */

#if FF_USE_LFN && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT-LFN: Create a Numbered SFN                                        */
/*-----------------------------------------------------------------------*/

static void gen_numname(
  // Pointer to the buffer to store numbered SFN
  BYTE*        dst,
  // Pointer to SFN in directory form
  const BYTE*  src,
  // Pointer to LFN
  const WCHAR* lfn,
  // Sequence number
  UINT         seq) {
    BYTE  ns[8], c;
    UINT  i, j;
    WCHAR wc;
    DWORD sreg;

    // Prepare the SFN to be modified
    memcpy(dst, src, 11);

    // In case of many collisions, generate a hash number instead of sequential
    // number
    if (seq > 5) {
        sreg = seq;
        // Create a CRC as hash value
        while (*lfn) {
            wc = *lfn++;
            for (i = 0; i < 16; i++) {
                sreg   = (sreg << 1) + (wc & 1);
                wc   >>= 1;
                if (sreg & 0x10000) {
                    sreg ^= 0x11021;
                }
            }
        }
        seq = (UINT)sreg;
    }

    // Make suffix (~ + hexadecimal)

    i = 7;
    do {
        c    = (BYTE)((seq % 16) + '0');
        seq /= 16;
        if (c > '9') {
            c += 7;
        }
        ns[i--] = c;
    } while (i && seq);
    ns[i] = '~';

    // Append the suffix to the SFN body

    // Find the offset to append
    for (j = 0; j < i && dst[j] != ' '; j++) {
        // To avoid DBC break up
        if (dbc_1st(dst[j])) {
            if (j == i - 1) {
                break;
            }
            j++;
        }
    }
    // Append the suffix
    do {
        dst[j++] = (i < 8) ? ns[i++] : ' ';
    } while (j < 8);
}
#endif /* FF_USE_LFN && !FF_FS_READONLY */

#if FF_USE_LFN
/*-----------------------------------------------------------------------*/
/* FAT-LFN: Calculate checksum of an SFN entry                           */
/*-----------------------------------------------------------------------*/

static BYTE sum_sfn(
  // Pointer to the SFN entry
  const BYTE* dir) {
    BYTE sum = 0;
    UINT n   = 11;

    do {
        sum = (sum >> 1) + (sum << 7) + *dir++;
    } while (--n);
    return sum;
}

#endif /* FF_USE_LFN */

#if FF_FS_EXFAT
/*-----------------------------------------------------------------------*/
/* exFAT: Checksum                                                       */
/*-----------------------------------------------------------------------*/

// Get checksum of the directoly entry block
static WORD xdir_sum(
  // Directory entry block to be calculated
  const BYTE* dir) {
    UINT i, szblk;
    WORD sum;

    // Number of bytes of the entry block
    szblk = (dir[XDIR_NumSec] + 1) * SZDIRE;
    for (i = sum = 0; i < szblk; i++) {
        // Skip 2-byte sum field
        if (i == XDIR_SetSum) {
            i++;
        }
        else {
            sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + dir[i];
        }
    }
    return sum;
}

// Get check sum (to be used as hash) of the file name
static WORD xname_sum(
  // File name to be calculated
  const WCHAR* name) {
    WCHAR chr;
    WORD  sum = 0;

    while ((chr = *name++) != 0) {
        // File name needs to be up-case converted
        chr = (WCHAR)ff_wtoupper(chr);
        sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + (chr & 0xFF);
        sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + (chr >> 8);
    }
    return sum;
}

#    if !FF_FS_READONLY && FF_USE_MKFS
// Returns 32-bit checksum
static DWORD xsum32(
  // Byte to be calculated (byte-by-byte processing)
  BYTE  dat,
  // Previous sum value
  DWORD sum) {
    sum = ((sum & 1) ? 0x80000000 : 0) + (sum >> 1) + dat;
    return sum;
}
#    endif

/*------------------------------------*/
/* exFAT: Get a directory entry block */
/*------------------------------------*/

// FR_INT_ERR: invalid entry block
static FRESULT load_xdir(
  // Reading directory object pointing top of the entry block to load
  DIR* dp) {
    FRESULT res;
    UINT    i, sz_ent;
    // Pointer to the on-memory directory entry block 85+C0+C1s
    BYTE*   dirb = dp->obj.fs->dirbuf;

    // Load file directory entry

    res          = move_window(dp->obj.fs, dp->sect);
    if (res != FR_OK) {
        return res;
    }
    // Invalid order
    if (dp->dir[XDIR_Type] != ET_FILEDIR) {
        return FR_INT_ERR;
    }
    memcpy(dirb + 0 * SZDIRE, dp->dir, SZDIRE);
    sz_ent = (dirb[XDIR_NumSec] + 1) * SZDIRE;
    if (sz_ent < 3 * SZDIRE || sz_ent > 19 * SZDIRE) {
        return FR_INT_ERR;
    }

    // Load stream extension entry

    res = dir_next(dp, 0);
    // It cannot be
    if (res == FR_NO_FILE) {
        res = FR_INT_ERR;
    }
    if (res != FR_OK) {
        return res;
    }
    res = move_window(dp->obj.fs, dp->sect);
    if (res != FR_OK) {
        return res;
    }
    // Invalid order
    if (dp->dir[XDIR_Type] != ET_STREAM) {
        return FR_INT_ERR;
    }
    memcpy(dirb + 1 * SZDIRE, dp->dir, SZDIRE);
    if (MAXDIRB(dirb[XDIR_NumName]) > sz_ent) {
        return FR_INT_ERR;
    }

    // Load file name entries

    // Name offset to load
    i = 2 * SZDIRE;
    do {
        res = dir_next(dp, 0);
        // It cannot be
        if (res == FR_NO_FILE) {
            res = FR_INT_ERR;
        }
        if (res != FR_OK) {
            return res;
        }
        res = move_window(dp->obj.fs, dp->sect);
        if (res != FR_OK) {
            return res;
        }
        // Invalid order
        if (dp->dir[XDIR_Type] != ET_FILENAME) {
            return FR_INT_ERR;
        }
        if (i < MAXDIRB(FF_MAX_LFN)) {
            memcpy(dirb + i, dp->dir, SZDIRE);
        }
    } while ((i += SZDIRE) < sz_ent);

    // Sanity check (do it for only accessible object)

    if (i <= MAXDIRB(FF_MAX_LFN)) {
        if (xdir_sum(dirb) != ld_word(dirb + XDIR_SetSum)) {
            return FR_INT_ERR;
        }
    }
    return FR_OK;
}

/*------------------------------------------------------------------*/
/* exFAT: Initialize object allocation info with loaded entry block */
/*------------------------------------------------------------------*/

static void init_alloc_info(
  // Filesystem object
  FATFS*   fs,
  // Object allocation information to be initialized
  FFOBJID* obj) {
    // Start cluster
    obj->sclust  = ld_dword(fs->dirbuf + XDIR_FstClus);
    // Size
    obj->objsize = ld_qword(fs->dirbuf + XDIR_FileSize);
    // Allocation status
    obj->stat    = fs->dirbuf[XDIR_GenFlags] & 2;
    // No last fragment info
    obj->n_frag  = 0;
}

#    if !FF_FS_READONLY || FF_FS_RPATH != 0
/*------------------------------------------------*/
/* exFAT: Load the object's directory entry block */
/*------------------------------------------------*/

static FRESULT load_obj_xdir(
  // Blank directory object to be used to access containing directory
  DIR*           dp,
  // Object with its containing directory information
  const FFOBJID* obj) {
    FRESULT res;

    // Open object containing directory

    dp->obj.fs      = obj->fs;
    dp->obj.sclust  = obj->c_scl;
    dp->obj.stat    = (BYTE)obj->c_size;
    dp->obj.objsize = obj->c_size & 0xFFFFFF00;
    dp->obj.n_frag  = 0;
    dp->blk_ofs     = obj->c_ofs;

    // Goto object's entry block
    res             = dir_sdi(dp, dp->blk_ofs);
    if (res == FR_OK) {
        // Load the object's entry block
        res = load_xdir(dp);
    }
    return res;
}
#    endif

#    if !FF_FS_READONLY
//----------------------------------------

// exFAT: Store the directory entry block

//----------------------------------------

static FRESULT store_xdir(
  // Pointer to the directory object
  DIR* dp) {
    FRESULT res;
    UINT    nent;
    // Pointer to the directory entry block 85+C0+C1s
    BYTE*   dirb = dp->obj.fs->dirbuf;

    // Create set sum

    st_word(dirb + XDIR_SetSum, xdir_sum(dirb));
    nent = dirb[XDIR_NumSec] + 1;

    // Store the directory entry block to the directory

    res  = dir_sdi(dp, dp->blk_ofs);
    while (res == FR_OK) {
        res = move_window(dp->obj.fs, dp->sect);
        if (res != FR_OK) {
            break;
        }
        memcpy(dp->dir, dirb, SZDIRE);
        dp->obj.fs->wflag = 1;
        if (--nent == 0) {
            break;
        }
        dirb += SZDIRE;
        res   = dir_next(dp, 0);
    }
    return (res == FR_OK || res == FR_DISK_ERR) ? res : FR_INT_ERR;
}

//-------------------------------------------

// exFAT: Create a new directory entry block

//-------------------------------------------

static void create_xdir(
  // Pointer to the directory entry block buffer
  BYTE*        dirb,
  // Pointer to the object name
  const WCHAR* lfn) {
    UINT  i;
    BYTE  nc1, nlen;
    WCHAR wc;

    // Create file-directory and stream-extension entry

    memset(dirb, 0, 2 * SZDIRE);
    dirb[0 * SZDIRE + XDIR_Type] = ET_FILEDIR;
    dirb[1 * SZDIRE + XDIR_Type] = ET_STREAM;

    // Create file-name entries

    // Top of file_name entries
    i                            = SZDIRE * 2;
    nlen = nc1 = 0;
    wc         = 1;
    do {
        dirb[i++] = ET_FILENAME;
        dirb[i++] = 0;
        // Fill name field
        do {
            // Get a character if exist
            if (wc != 0 && (wc = lfn[nlen]) != 0) {
                nlen++;
            }
            // Store it
            st_word(dirb + i, wc);
            i += 2;
        } while (i % SZDIRE != 0);
        nc1++;
        // Fill next entry if any char follows
    } while (lfn[nlen]);

    // Set name length
    dirb[XDIR_NumName] = nlen;
    // Set secondary count (C0 + C1s)
    dirb[XDIR_NumSec]  = 1 + nc1;
    // Set name hash
    st_word(dirb + XDIR_NameHash, xname_sum(lfn));
}

#    endif /* !FF_FS_READONLY */
#endif     /* FF_FS_EXFAT */

#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 || FF_USE_LABEL || FF_FS_EXFAT
/*-----------------------------------------------------------------------*/
/* Read an object from the directory                                     */
/*-----------------------------------------------------------------------*/

#    define DIR_READ_FILE(dp)  dir_read(dp, 0)
#    define DIR_READ_LABEL(dp) dir_read(dp, 1)

static FRESULT dir_read(
  // Pointer to the directory object
  DIR* dp,
  // Filtered by 0:file/directory or 1:volume label
  int  vol) {
    FRESULT res = FR_NO_FILE;
    FATFS*  fs  = dp->obj.fs;
    BYTE    attr, b;
#    if FF_USE_LFN
    BYTE ord = 0xFF, sum = 0xFF;
#    endif

    while (dp->sect) {
        res = move_window(fs, dp->sect);
        if (res != FR_OK) {
            break;
        }
        // Test for the entry type
        b = dp->dir[DIR_Name];
        if (b == 0) {
            // Reached to end of the directory
            res = FR_NO_FILE;
            break;
        }
#    if FF_FS_EXFAT
        // On the exFAT volume
        if (fs->fs_type == FS_EXFAT) {
            if (FF_USE_LABEL && vol) {
                // Volume label entry?
                if (b == ET_VLABEL) {
                    break;
                }
            }
            else {
                // Start of the file entry block?
                if (b == ET_FILEDIR) {
                    // Get location of the block
                    dp->blk_ofs = dp->dptr;
                    // Load the entry block
                    res         = load_xdir(dp);
                    if (res == FR_OK) {
                        // Get attribute
                        dp->obj.attr = fs->dirbuf[XDIR_Attr] & AM_MASK;
                    }
                    break;
                }
            }
        }
        else
#    endif
        // On the FAT/FAT32 volume
        {
            // Get attribute
            dp->obj.attr = attr = dp->dir[DIR_Attr] & AM_MASK;
#    if FF_USE_LFN /* LFN configuration */
            if (b == DDEM || b == '.'
                || (int)((attr & ~AM_ARC) == AM_VOL)
                     != vol) { /* An entry without valid data */
                ord = 0xFF;
            }
            else {
                // An LFN entry is found
                if (attr == AM_LFN) {
                    // Is it start of an LFN sequence?
                    if (b & LLEF) {
                        sum          = dp->dir[LDIR_Chksum];
                        b           &= (BYTE)~LLEF;
                        ord          = b;
                        dp->blk_ofs  = dp->dptr;
                    }
                    // Check LFN validity and capture it

                    ord = (b == ord && sum == dp->dir[LDIR_Chksum]
                           && pick_lfn(fs->lfnbuf, dp->dir))
                          ? ord - 1
                          : 0xFF;
                    // An SFN entry is found
                }
                else {
                    // Is there a valid LFN?
                    if (ord != 0 || sum != sum_sfn(dp->dir)) {
                        // It has no LFN.
                        dp->blk_ofs = 0xFFFFFFFF;
                    }
                    break;
                }
            }
            // Non LFN configuration
#    else
            // Is it a valid entry?
            if (b != DDEM && b != '.' && attr != AM_LFN
                && (int)((attr & ~AM_ARC) == AM_VOL) == vol) {
                break;
            }
#    endif
        }
        // Next entry
        res = dir_next(dp, 0);
        if (res != FR_OK) {
            break;
        }
    }

    // Terminate the read operation on error or EOT
    if (res != FR_OK) {
        dp->sect = 0;
    }
    return res;
}

#endif /* FF_FS_MINIMIZE <= 1 || FF_USE_LABEL || FF_FS_RPATH >= 2 */

/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

// FR_OK(0):succeeded, !=0:error
static FRESULT dir_find(
  // Pointer to the directory object with the file name
  DIR* dp) {
    FRESULT res;
    FATFS*  fs = dp->obj.fs;
    BYTE    c;
#if FF_USE_LFN
    BYTE a, ord, sum;
#endif

    // Rewind directory object
    res = dir_sdi(dp, 0);
    if (res != FR_OK) {
        return res;
    }
#if FF_FS_EXFAT
    // On the exFAT volume
    if (fs->fs_type == FS_EXFAT) {
        BYTE nc;
        UINT di, ni;
        // Hash value of the name to find
        WORD hash = xname_sum(fs->lfnbuf);

        // Read an item
        while ((res = DIR_READ_FILE(dp)) == FR_OK) {
#    if FF_MAX_LFN < 255
            // Skip comparison if inaccessible object name
            if (fs->dirbuf[XDIR_NumName] > FF_MAX_LFN) {
                continue;
            }
#    endif
            // Skip comparison if hash mismatched
            if (ld_word(fs->dirbuf + XDIR_NameHash) != hash) {
                continue;
            }
            // Compare the name
            for (nc = fs->dirbuf[XDIR_NumName], di = SZDIRE * 2, ni = 0; nc;
                 nc--, di += 2, ni++) {
                if ((di % SZDIRE) == 0) {
                    di += 2;
                }
                if (ff_wtoupper(ld_word(fs->dirbuf + di))
                    != ff_wtoupper(fs->lfnbuf[ni])) {
                    break;
                }
            }
            // Name matched?
            if (nc == 0 && !fs->lfnbuf[ni]) {
                break;
            }
        }
        return res;
    }
#endif
    // On the FAT/FAT32 volume

#if FF_USE_LFN
    // Reset LFN sequence
    ord = sum   = 0xFF;
    dp->blk_ofs = 0xFFFFFFFF;
#endif
    do {
        res = move_window(fs, dp->sect);
        if (res != FR_OK) {
            break;
        }
        c = dp->dir[DIR_Name];
        // Reached to end of table
        if (c == 0) {
            res = FR_NO_FILE;
            break;
        }
        // LFN configuration
#if FF_USE_LFN
        dp->obj.attr = a = dp->dir[DIR_Attr] & AM_MASK;
        // An entry without valid data
        if (c == DDEM || ((a & AM_VOL) && a != AM_LFN)) {
            // Reset LFN sequence
            ord         = 0xFF;
            dp->blk_ofs = 0xFFFFFFFF;
        }
        else {
            // An LFN entry is found
            if (a == AM_LFN) {
                if (!(dp->fn[NSFLAG] & NS_NOLFN)) {
                    // Is it start of LFN sequence?
                    if (c & LLEF) {
                        sum          = dp->dir[LDIR_Chksum];
                        // LFN start order
                        c           &= (BYTE)~LLEF;
                        ord          = c;
                        // Start offset of LFN
                        dp->blk_ofs  = dp->dptr;
                    }
                    // Check validity of the LFN entry and compare it with given
                    // name

                    ord = (c == ord && sum == dp->dir[LDIR_Chksum]
                           && cmp_lfn(fs->lfnbuf, dp->dir))
                          ? ord - 1
                          : 0xFF;
                }
                // An SFN entry is found
            }
            else {
                // LFN matched?
                if (ord == 0 && sum == sum_sfn(dp->dir)) {
                    break;
                }
                // SFN matched?
                if (!(dp->fn[NSFLAG] & NS_LOSS)
                    && !memcmp(dp->dir, dp->fn, 11)) {
                    break;
                }
                // Reset LFN sequence
                ord         = 0xFF;
                dp->blk_ofs = 0xFFFFFFFF;
            }
        }
        // Non LFN configuration
#else
        dp->obj.attr = dp->dir[DIR_Attr] & AM_MASK;
        // Is it a valid entry?
        if (!(dp->dir[DIR_Attr] & AM_VOL) && !memcmp(dp->dir, dp->fn, 11)) {
            break;
        }
#endif
        // Next entry
        res = dir_next(dp, 0);
    } while (res == FR_OK);

    return res;
}

#if !FF_FS_READONLY
//-----------------------------------------------------------------------

// Register an object to the directory

//-----------------------------------------------------------------------

// FR_OK:succeeded, FR_DENIED:no free entry or too many SFN collision,
// FR_DISK_ERR:disk error
static FRESULT dir_register(
  // Target directory with object name to be created
  DIR* dp) {
    FRESULT res;
    FATFS*  fs = dp->obj.fs;
    // LFN configuration
#    if FF_USE_LFN
    UINT n, len, n_ent;
    BYTE sn[12], sum;

    // Check name validity
    if (dp->fn[NSFLAG] & (NS_DOT | NS_NONAME)) {
        return FR_INVALID_NAME;
    }
    // Get lfn length
    for (len = 0; fs->lfnbuf[len]; len++)
        ;

#        if FF_FS_EXFAT
    // On the exFAT volume
    if (fs->fs_type == FS_EXFAT) {
        // Number of entries to allocate (85+C0+C1s)
        n_ent = (len + 14) / 15 + 2;
        // Allocate directory entries
        res   = dir_alloc(dp, n_ent);
        if (res != FR_OK) {
            return res;
        }
        // Set the allocated entry block offset
        dp->blk_ofs = dp->dptr - SZDIRE * (n_ent - 1);

        // Has the directory been stretched by new allocation?
        if (dp->obj.stat & 4) {
            dp->obj.stat &= ~4;
            // Fill the first fragment on the FAT if needed
            res           = fill_first_frag(&dp->obj);
            if (res != FR_OK) {
                return res;
            }
            // Fill the last fragment on the FAT if needed
            res = fill_last_frag(&dp->obj, dp->clust, 0xFFFFFFFF);
            if (res != FR_OK) {
                return res;
            }
            // Is it a sub-directory?
            if (dp->obj.sclust != 0) {
                DIR dj;

                // Load the object status
                res = load_obj_xdir(&dj, &dp->obj);
                if (res != FR_OK) {
                    return res;
                }
                // Increase the directory size by cluster size
                dp->obj.objsize += (DWORD)fs->csize * SS(fs);
                st_qword(fs->dirbuf + XDIR_FileSize, dp->obj.objsize);
                st_qword(fs->dirbuf + XDIR_ValidFileSize, dp->obj.objsize);
                // Update the allocation status
                fs->dirbuf[XDIR_GenFlags] = dp->obj.stat | 1;
                // Store the object status
                res                       = store_xdir(&dj);
                if (res != FR_OK) {
                    return res;
                }
            }
        }

        // Create on-memory directory block to be written later
        create_xdir(fs->dirbuf, fs->lfnbuf);
        return FR_OK;
    }
#        endif
    // On the FAT/FAT32 volume

    memcpy(sn, dp->fn, 12);
    // When LFN is out of 8.3 format, generate a numbered name
    if (sn[NSFLAG] & NS_LOSS) {
        // Find only SFN
        dp->fn[NSFLAG] = NS_NOLFN;
        for (n = 1; n < 100; n++) {
            // Generate a numbered name
            gen_numname(dp->fn, sn, fs->lfnbuf, n);
            // Check if the name collides with existing SFN
            res = dir_find(dp);
            if (res != FR_OK) {
                break;
            }
        }
        // Abort if too many collisions
        if (n == 100) {
            return FR_DENIED;
        }
        // Abort if the result is other than 'not collided'
        if (res != FR_NO_FILE) {
            return res;
        }
        dp->fn[NSFLAG] = sn[NSFLAG];
    }

    // Create an SFN with/without LFNs.

    // Number of entries to allocate
    n_ent = (sn[NSFLAG] & NS_LFN) ? (len + 12) / 13 + 1 : 1;
    // Allocate entries
    res   = dir_alloc(dp, n_ent);
    // Set LFN entry if needed
    if (res == FR_OK && --n_ent) {
        res = dir_sdi(dp, dp->dptr - n_ent * SZDIRE);
        if (res == FR_OK) {
            // Checksum value of the SFN tied to the LFN
            sum = sum_sfn(dp->fn);
            // Store LFN entries in bottom first
            do {
                res = move_window(fs, dp->sect);
                if (res != FR_OK) {
                    break;
                }
                put_lfn(fs->lfnbuf, dp->dir, (BYTE)n_ent, sum);
                fs->wflag = 1;
                // Next entry
                res       = dir_next(dp, 0);
            } while (res == FR_OK && --n_ent);
        }
    }

// Non LFN configuration
#    else
    // Allocate an entry for SFN
    res = dir_alloc(dp, 1);

#    endif

    // Set SFN entry

    if (res == FR_OK) {
        res = move_window(fs, dp->sect);
        if (res == FR_OK) {
            // Clean the entry
            memset(dp->dir, 0, SZDIRE);
            // Put SFN
            memcpy(dp->dir + DIR_Name, dp->fn, 11);
#    if FF_USE_LFN
            // Put NT flag
            dp->dir[DIR_NTres] = dp->fn[NSFLAG] & (NS_BODY | NS_EXT);
#    endif
            fs->wflag = 1;
        }
    }

    return res;
}

// !FF_FS_READONLY
#endif

#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0
//-----------------------------------------------------------------------

// Remove an object from the directory

//-----------------------------------------------------------------------

// FR_OK:Succeeded, FR_DISK_ERR:A disk error
static FRESULT dir_remove(
  // Directory object pointing the entry to be removed
  DIR* dp) {
    FRESULT res;
    FATFS*  fs = dp->obj.fs;
    // LFN configuration
#    if FF_USE_LFN
    DWORD last = dp->dptr;

    // Goto top of the entry block if LFN is exist
    res        = (dp->blk_ofs == 0xFFFFFFFF) ? FR_OK : dir_sdi(dp, dp->blk_ofs);
    if (res == FR_OK) {
        do {
            res = move_window(fs, dp->sect);
            if (res != FR_OK) {
                break;
            }
            // On the exFAT volume
            if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
                // Clear the entry InUse flag.
                dp->dir[XDIR_Type] &= 0x7F;
                // On the FAT/FAT32 volume
            }
            else {
                // Mark the entry 'deleted'.
                dp->dir[DIR_Name] = DDEM;
            }
            fs->wflag = 1;
            // If reached last entry then all entries of the object has been
            // deleted.
            if (dp->dptr >= last) {
                break;
            }
            // Next entry
            res = dir_next(dp, 0);
        } while (res == FR_OK);
        if (res == FR_NO_FILE) {
            res = FR_INT_ERR;
        }
    }
    // Non LFN configuration
#    else

    res = move_window(fs, dp->sect);
    if (res == FR_OK) {
        // Mark the entry 'deleted'.
        dp->dir[DIR_Name] = DDEM;
        fs->wflag         = 1;
    }
#    endif

    return res;
}

// !FF_FS_READONLY && FF_FS_MINIMIZE == 0
#endif

#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2
//-----------------------------------------------------------------------

// Get file information from directory entry

//-----------------------------------------------------------------------

static void get_fileinfo(
  // Pointer to the directory object
  DIR*     dp,
  // Pointer to the file information to be filled
  FILINFO* fno) {
    UINT si, di;
#    if FF_USE_LFN
    BYTE   lcf;
    WCHAR  wc, hs;
    FATFS* fs = dp->obj.fs;
    UINT   nw;
#    else
    TCHAR c;
#    endif

    // Invaidate file info
    fno->fname[0] = 0;
    // Exit if read pointer has reached end of directory
    if (dp->sect == 0) {
        return;
    }

// LFN configuration
#    if FF_USE_LFN
#        if FF_FS_EXFAT
    // exFAT volume
    if (fs->fs_type == FS_EXFAT) {
        UINT nc = 0;

        // 1st C1 entry in the entry block
        si      = SZDIRE * 2;
        di      = 0;
        hs      = 0;
        while (nc < fs->dirbuf[XDIR_NumName]) {
            // Truncated directory block?
            if (si >= MAXDIRB(FF_MAX_LFN)) {
                di = 0;
                break;
            }
            // Skip entry type field
            if ((si % SZDIRE) == 0) {
                si += 2;
            }
            // Get a character
            wc  = ld_word(fs->dirbuf + si);
            si += 2;
            nc++;
            // Is it a surrogate?
            if (hs == 0 && IsSurrogate(wc)) {
                // Get low surrogate
                hs = wc;
                continue;
            }
            // Store it in API encoding
            nw
              = put_utf((DWORD)hs << 16 | wc, &fno->fname[di], FF_LFN_BUF - di);
            // Buffer overflow or wrong char?
            if (nw == 0) {
                di = 0;
                break;
            }
            di += nw;
            hs  = 0;
        }
        // Broken surrogate pair?
        if (hs != 0) {
            di = 0;
        }
        // Inaccessible object name?
        if (di == 0) {
            fno->fname[di++] = '\?';
        }
        // Terminate the name
        fno->fname[di]  = 0;
        // exFAT does not support SFN
        fno->altname[0] = 0;

        // Attribute
        fno->fattrib    = fs->dirbuf[XDIR_Attr] & AM_MASKX;
        // Size
        fno->fsize
          = (fno->fattrib & AM_DIR) ? 0 : ld_qword(fs->dirbuf + XDIR_FileSize);
        // Time
        fno->ftime = ld_word(fs->dirbuf + XDIR_ModTime + 0);
        // Date
        fno->fdate = ld_word(fs->dirbuf + XDIR_ModTime + 2);
        return;
    }
    else
#        endif
    // FAT/FAT32 volume
    {
        // Get LFN if available
        if (dp->blk_ofs != 0xFFFFFFFF) {
            si = di = 0;
            hs      = 0;
            while (fs->lfnbuf[si] != 0) {
                // Get an LFN character (UTF-16)
                wc = fs->lfnbuf[si++];
                // Is it a surrogate?
                if (hs == 0 && IsSurrogate(wc)) {
                    // Get low surrogate
                    hs = wc;
                    continue;
                }
                // Store it in API encoding
                nw = put_utf((DWORD)hs << 16 | wc, &fno->fname[di],
                             FF_LFN_BUF - di);
                // Buffer overflow or wrong char?
                if (nw == 0) {
                    di = 0;
                    break;
                }
                di += nw;
                hs  = 0;
            }
            // Broken surrogate pair?
            if (hs != 0) {
                di = 0;
            }
            // Terminate the LFN (null string means LFN is invalid)
            fno->fname[di] = 0;
        }
    }

    si = di = 0;
    // Get SFN from SFN entry
    while (si < 11) {
        // Get a char
        wc = dp->dir[si++];
        // Skip padding spaces
        if (wc == ' ') {
            continue;
        }
        // Restore replaced DDEM character
        if (wc == RDDEM) {
            wc = DDEM;
        }
        // Insert a . if extension is exist
        if (si == 9 && di < FF_SFN_BUF) {
            fno->altname[di++] = '.';
        }
        // Unicode output
#        if FF_LFN_UNICODE >= 1
        // Make a DBC if needed
        if (dbc_1st((BYTE)wc) && si != 8 && si != 11 && dbc_2nd(dp->dir[si])) {
            wc = wc << 8 | dp->dir[si++];
        }
        // ANSI/OEM -> Unicode
        wc = ff_oem2uni(wc, CODEPAGE);
        // Wrong char in the current code page?
        if (wc == 0) {
            di = 0;
            break;
        }
        // Store it in API encoding
        nw = put_utf(wc, &fno->altname[di], FF_SFN_BUF - di);
        // Buffer overflow?
        if (nw == 0) {
            di = 0;
            break;
        }
        di += nw;
        // ANSI/OEM output
#        else
        // Store it without any conversion
        fno->altname[di++] = (TCHAR)wc;
#        endif
    }
    // Terminate the SFN  (null string means SFN is invalid)
    fno->altname[di] = 0;

    // If LFN is invalid, altname[] needs to be copied to fname[]
    if (fno->fname[0] == 0) {
        // If LFN and SFN both are invalid, this object is inaccessible
        if (di == 0) {
            fno->fname[di++] = '\?';
        }
        else {
            // Copy altname[] to fname[] with case information
            for (si = di = 0, lcf = NS_BODY; fno->altname[si]; si++, di++) {
                wc = (WCHAR)fno->altname[si];
                if (wc == '.') {
                    lcf = NS_EXT;
                }
                if (IsUpper(wc) && (dp->dir[DIR_NTres] & lcf)) {
                    wc += 0x20;
                }
                fno->fname[di] = (TCHAR)wc;
            }
        }
        // Terminate the LFN
        fno->fname[di] = 0;
        // Altname is not needed if neither LFN nor case info is exist.
        if (!dp->dir[DIR_NTres]) {
            fno->altname[0] = 0;
        }
    }

// Non-LFN configuration
#    else
    si = di = 0;
    // Copy name body and extension
    while (si < 11) {
        c = (TCHAR)dp->dir[si++];
        // Skip padding spaces
        if (c == ' ') {
            continue;
        }
        // Restore replaced DDEM character
        if (c == RDDEM) {
            c = DDEM;
        }
        // Insert a . if extension is exist
        if (si == 9) {
            fno->fname[di++] = '.';
        }
        fno->fname[di++] = c;
    }
    // Terminate the SFN
    fno->fname[di] = 0;
#    endif

    // Attribute
    fno->fattrib = dp->dir[DIR_Attr] & AM_MASK;
    // Size
    fno->fsize   = ld_dword(dp->dir + DIR_FileSize);
    // Time
    fno->ftime   = ld_word(dp->dir + DIR_ModTime + 0);
    // Date
    fno->fdate   = ld_word(dp->dir + DIR_ModTime + 2);
}

// FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2
#endif

#if FF_USE_FIND && FF_FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Pattern matching                                                      */
/*-----------------------------------------------------------------------*/

// Maximum number of wildcard terms in the pattern to limit recursion
#    define FIND_RECURS 4

// Get a character and advance ptr
static DWORD get_achar(
  // Pointer to pointer to the ANSI/OEM or Unicode string
  const TCHAR** ptr) {
    DWORD chr;

// Unicode input
#    if FF_USE_LFN && FF_LFN_UNICODE >= 1
    chr = tchar2uni(ptr);
    // Wrong UTF encoding is recognized as end of the string
    if (chr == 0xFFFFFFFF) {
        chr = 0;
    }
    chr = ff_wtoupper(chr);

// ANSI/OEM input
#    else
    // Get a byte
    chr            = (BYTE) * (*ptr)++;
    // To upper ASCII char
    if (IsLower(chr)) {
        chr -= 0x20;
    }
#        if FF_CODE_PAGE == 0
    // To upper SBCS extended char
    if (ExCvt && chr >= 0x80) {
        chr = ExCvt[chr - 0x80];
    }
#        elif FF_CODE_PAGE < 900
    // To upper SBCS extended char
    if (chr >= 0x80) {
        chr = ExCvt[chr - 0x80];
    }
#        endif
#        if FF_CODE_PAGE == 0 || FF_CODE_PAGE >= 900
    // Get DBC 2nd byte if needed
    if (dbc_1st((BYTE)chr)) {
        chr = dbc_2nd((BYTE) * *ptr) ? chr << 8 | (BYTE) * (*ptr)++ : 0;
    }
#        endif

#    endif
    return chr;
}

// 0:mismatched, 1:matched
static int pattern_match(
  // Matching pattern
  const TCHAR* pat,
  // String to be tested
  const TCHAR* nam,
  // Number of pre-skip chars (number of ?s, b8:infinite (* specified))
  UINT         skip,
  // Recursion count
  UINT         recur) {
    const TCHAR* pptr;
    const TCHAR* nptr;
    DWORD        pchr, nchr;
    UINT         sk;

    // Pre-skip name chars
    while ((skip & 0xFF) != 0) {
        // Branch mismatched if less name chars
        if (!get_achar(&nam)) {
            return 0;
        }
        skip--;
    }
    // Matched? (short circuit)
    if (*pat == 0 && skip) {
        return 1;
    }

    do {
        // Top of pattern and name to match
        pptr = pat;
        nptr = nam;
        for (;;) {
            // Wildcard term?
            if (*pptr == '\?' || *pptr == '*') {
                // Too many wildcard terms?
                if (recur == 0) {
                    return 0;
                }
                sk = 0;
                // Analyze the wildcard term
                do {
                    if (*pptr++ == '\?') {
                        sk++;
                    }
                    else {
                        sk |= 0x100;
                    }
                } while (*pptr == '\?' || *pptr == '*');
                // Test new branch (recursive call)
                if (pattern_match(pptr, nptr, sk, recur - 1)) {
                    return 1;
                }
                // Branch mismatched
                nchr = *nptr;
                break;
            }
            // Get a pattern char
            pchr = get_achar(&pptr);
            // Get a name char
            nchr = get_achar(&nptr);
            // Branch mismatched?
            if (pchr != nchr) {
                break;
            }
            // Branch matched? (matched at end of both strings)
            if (pchr == 0) {
                return 1;
            }
        }
        // nam++
        get_achar(&nam);
        // Retry until end of name if infinite search is specified
    } while (skip && nchr);

    return 0;
}

// FF_USE_FIND && FF_FS_MINIMIZE <= 1
#endif

//-----------------------------------------------------------------------

// Pick a top segment and create the object name in directory form

//-----------------------------------------------------------------------

// FR_OK: successful, FR_INVALID_NAME: could not create
static FRESULT create_name(
  // Pointer to the directory object
  DIR*          dp,
  // Pointer to pointer to the segment in the path string
  const TCHAR** path) {
    // LFN configuration
#if FF_USE_LFN
    BYTE         b, cf;
    WCHAR        wc;
    WCHAR*       lfn;
    const TCHAR* p;
    DWORD        uc;
    UINT         i, ni, si, di;

    // Create LFN into LFN working buffer

    p   = *path;
    lfn = dp->obj.fs->lfnbuf;
    di  = 0;
    for (;;) {
        // Get a character
        uc = tchar2uni(&p);
        // Invalid code or UTF decode error
        if (uc == 0xFFFFFFFF) {
            return FR_INVALID_NAME;
        }
        // Store high surrogate if needed
        if (uc >= 0x10000) {
            lfn[di++] = (WCHAR)(uc >> 16);
        }
        wc = (WCHAR)uc;
        // Break if end of the path or a separator is found
        if (wc < ' ' || IsSeparator(wc)) {
            break;
        }
        // Reject illegal characters for LFN
        if (wc < 0x80 && strchr("*:<>|\"\?\x7F", (int)wc)) {
            return FR_INVALID_NAME;
        }
        // Reject too long name
        if (di >= FF_MAX_LFN) {
            return FR_INVALID_NAME;
        }
        // Store the Unicode character
        lfn[di++] = wc;
    }
    // Stopped at end of the path?
    if (wc < ' ') {
        // Last segment
        cf = NS_LAST;
        // Stopped at a separator
    }
    else {
        // Skip duplicated separators if exist
        while (IsSeparator(*p)) {
            p++;
        }
        // Next segment may follow
        cf = 0;
        // Ignore terminating separator
        if (IsTerminator(*p)) {
            cf = NS_LAST;
        }
    }
    // Return pointer to the next segment
    *path = p;

#    if FF_FS_RPATH != 0
    if ((di == 1 && lfn[di - 1] == '.') ||
        // Is this segment a dot name?
        (di == 2 && lfn[di - 1] == '.' && lfn[di - 2] == '.')) {
        lfn[di] = 0;
        // Create dot name for SFN entry
        for (i = 0; i < 11; i++) {
            dp->fn[i] = (i < di) ? '.' : ' ';
        }
        // This is a dot entry
        dp->fn[i] = cf | NS_DOT;
        return FR_OK;
    }
#    endif
    // Snip off trailing spaces and dots if exist
    while (di) {
        wc = lfn[di - 1];
        if (wc != ' ' && wc != '.') {
            break;
        }
        di--;
    }
    // LFN is created into the working buffer
    lfn[di] = 0;
    // Reject null name
    if (di == 0) {
        return FR_INVALID_NAME;
    }

    // Create SFN in directory form

    // Remove leading spaces
    for (si = 0; lfn[si] == ' '; si++)
        ;
    // Is there any leading space or dot?
    if (si > 0 || lfn[si] == '.') {
        cf |= NS_LOSS | NS_LFN;
    }
    // Find last dot (di<=si: no extension)
    while (di > 0 && lfn[di - 1] != '.') {
        di--;
    }

    memset(dp->fn, ' ', 11);
    i = b = 0;
    ni    = 8;
    for (;;) {
        // Get an LFN character
        wc = lfn[si++];
        // Break on end of the LFN
        if (wc == 0) {
            break;
        }
        // Remove embedded spaces and dots
        if (wc == ' ' || (wc == '.' && si != di)) {
            cf |= NS_LOSS | NS_LFN;
            continue;
        }

        // End of field?
        if (i >= ni || si == di) {
            // Name extension overflow?
            if (ni == 11) {
                cf |= NS_LOSS | NS_LFN;
                break;
            }
            // Name body overflow?
            if (si != di) {
                cf |= NS_LOSS | NS_LFN;
            }
            // No name extension?
            if (si > di) {
                break;
            }
            // Enter name extension
            si   = di;
            i    = 8;
            ni   = 11;
            b  <<= 2;
            continue;
        }

        // Is this an extended character?
        if (wc >= 0x80) {
            // LFN entry needs to be created
            cf |= NS_LFN;
#    if FF_CODE_PAGE == 0
            // In SBCS cfg
            if (ExCvt) {
                // Unicode ==> ANSI/OEM code
                wc = ff_uni2oem(wc, CODEPAGE);
                // Convert extended character to upper (SBCS)
                if (wc & 0x80) {
                    wc = ExCvt[wc & 0x7F];
                }
                // In DBCS cfg
            }
            else {
                // Unicode ==> Up-convert ==> ANSI/OEM code
                wc = ff_uni2oem(ff_wtoupper(wc), CODEPAGE);
            }
            // In SBCS cfg
#    elif FF_CODE_PAGE < 900
            // Unicode ==> ANSI/OEM code
            wc = ff_uni2oem(wc, CODEPAGE);
            // Convert extended character to upper (SBCS)
            if (wc & 0x80) {
                wc = ExCvt[wc & 0x7F];
            }
            // In DBCS cfg
#    else
            // Unicode ==> Up-convert ==> ANSI/OEM code
            wc = ff_uni2oem(ff_wtoupper(wc), CODEPAGE);
#    endif
        }

        // Is this a DBC?
        if (wc >= 0x100) {
            // Field overflow?
            if (i >= ni - 1) {
                cf |= NS_LOSS | NS_LFN;
                // Next field
                i   = ni;
                continue;
            }
            // Put 1st byte
            dp->fn[i++] = (BYTE)(wc >> 8);
            // SBC
        }
        else {
            // Replace illegal characters for SFN
            if (wc == 0 || strchr("+,;=[]", (int)wc)) {
                // Lossy conversion
                wc  = '_';
                cf |= NS_LOSS | NS_LFN;
            }
            else {
                // ASCII upper case?
                if (IsUpper(wc)) {
                    b |= 2;
                }
                // ASCII lower case?
                if (IsLower(wc)) {
                    b  |= 1;
                    wc -= 0x20;
                }
            }
        }
        dp->fn[i++] = (BYTE)wc;
    }

    // If the first character collides with DDEM, replace it with RDDEM
    if (dp->fn[0] == DDEM) {
        dp->fn[0] = RDDEM;
    }

    // Shift capital flags if no extension
    if (ni == 8) {
        b <<= 2;
    }
    // LFN entry needs to be created if composite capitals
    if ((b & 0x0C) == 0x0C || (b & 0x03) == 0x03) {
        cf |= NS_LFN;
    }
    // When LFN is in 8.3 format without extended character, NT flags are
    // created
    if (!(cf & NS_LFN)) {
        // NT flag (Extension has small capital letters only)
        if (b & 0x01) {
            cf |= NS_EXT;
        }
        // NT flag (Body has small capital letters only)
        if (b & 0x04) {
            cf |= NS_BODY;
        }
    }

    // SFN is created into dp->fn[]
    dp->fn[NSFLAG] = cf;

    return FR_OK;

// FF_USE_LFN : Non-LFN configuration
#else
    BYTE        c, d;
    BYTE*       sfn;
    UINT        ni, si, i;
    const char* p;

    // Create file name in directory form

    p   = *path;
    sfn = dp->fn;
    memset(sfn, ' ', 11);
    si = i = 0;
    ni     = 8;
#    if FF_FS_RPATH != 0
    // Is this a dot entry?
    if (p[si] == '.') {
        for (;;) {
            c = (BYTE)p[si++];
            if (c != '.' || si >= 3) {
                break;
            }
            sfn[i++] = c;
        }
        if (!IsSeparator(c) && c > ' ') {
            return FR_INVALID_NAME;
        }
        // Return pointer to the next segment
        *path       = p + si;
        // Set last segment flag if end of the path
        sfn[NSFLAG] = (c <= ' ') ? NS_LAST | NS_DOT : NS_DOT;
        return FR_OK;
    }
#    endif
    for (;;) {
        // Get a byte
        c = (BYTE)p[si++];
        // Break if end of the path name
        if (c <= ' ') {
            break;
        }
        // Break if a separator is found
        if (IsSeparator(c)) {
            // Skip duplicated separator if exist
            while (IsSeparator(p[si])) {
                si++;
            }
            break;
        }
        // End of body or field overflow?
        if (c == '.' || i >= ni) {
            // Field overflow or invalid dot?
            if (ni == 11 || c != '.') {
                return FR_INVALID_NAME;
            }
            // Enter file extension field
            i  = 8;
            ni = 11;
            continue;
        }
#    if FF_CODE_PAGE == 0
        // Is SBC extended character?
        if (ExCvt && c >= 0x80) {
            // To upper SBC extended character
            c = ExCvt[c & 0x7F];
        }
#    elif FF_CODE_PAGE < 900
        // Is SBC extended character?
        if (c >= 0x80) {
            // To upper SBC extended character
            c = ExCvt[c & 0x7F];
        }
#    endif
        // Check if it is a DBC 1st byte
        if (dbc_1st(c)) {
            // Get 2nd byte
            d = (BYTE)p[si++];
            // Reject invalid DBC
            if (!dbc_2nd(d) || i >= ni - 1) {
                return FR_INVALID_NAME;
            }
            sfn[i++] = c;
            sfn[i++] = d;
            // SBC
        }
        else {
            // Reject illegal chrs for SFN
            if (strchr("*+,:;<=>[]|\"\?\x7F", (int)c)) {
                return FR_INVALID_NAME;
            }
            // To upper
            if (IsLower(c)) {
                c -= 0x20;
            }
            sfn[i++] = c;
        }
    }
    // Return pointer to the next segment
    *path = &p[si];
    // Reject nul string
    if (i == 0) {
        return FR_INVALID_NAME;
    }

    // If the first character collides with DDEM, replace it with RDDEM
    if (sfn[0] == DDEM) {
        sfn[0] = RDDEM;
    }
    // Set last segment flag if end of the path
    sfn[NSFLAG] = (c <= ' ' || p[si] <= ' ') ? NS_LAST : 0;

    return FR_OK;
    // FF_USE_LFN
#endif
}

//-----------------------------------------------------------------------

// Follow a file path

//-----------------------------------------------------------------------

// FR_OK(0): successful, !=0: error code
static FRESULT follow_path(
  // Directory object to return last directory and found object
  DIR*         dp,
  // Full-path string to find a file or directory
  const TCHAR* path) {
    FRESULT res;
    BYTE    ns;
    FATFS*  fs = dp->obj.fs;

#if FF_FS_RPATH != 0
    // Without heading separator
    if (!IsSeparator(*path)
        && (FF_STR_VOLUME_ID != 2 || !IsTerminator(*path))) {
        // Start at the current directory
        dp->obj.sclust = fs->cdir;
    }
    else
#endif
    // With heading separator
    {
        // Strip separators
        while (IsSeparator(*path)) {
            path++;
        }
        // Start from the root directory
        dp->obj.sclust = 0;
    }
#if FF_FS_EXFAT
    // Invalidate last fragment counter of the object
    dp->obj.n_frag = 0;
#    if FF_FS_RPATH != 0
    // exFAT: Retrieve the sub-directory's status
    if (fs->fs_type == FS_EXFAT && dp->obj.sclust) {
        DIR dj;

        dp->obj.c_scl  = fs->cdc_scl;
        dp->obj.c_size = fs->cdc_size;
        dp->obj.c_ofs  = fs->cdc_ofs;
        res            = load_obj_xdir(&dj, &dp->obj);
        if (res != FR_OK) {
            return res;
        }
        dp->obj.objsize = ld_dword(fs->dirbuf + XDIR_FileSize);
        dp->obj.stat    = fs->dirbuf[XDIR_GenFlags] & 2;
    }
#    endif
#endif

    // Null path name is the origin directory itself
    if ((UINT)*path < ' ') {
        dp->fn[NSFLAG] = NS_NONAME;
        res            = dir_sdi(dp, 0);

        // Follow path
    }
    else {
        for (;;) {
            // Get a segment name of the path
            res = create_name(dp, &path);
            if (res != FR_OK) {
                break;
            }
            // Find an object with the segment name
            res = dir_find(dp);
            ns  = dp->fn[NSFLAG];
            // Failed to find the object
            if (res != FR_OK) {
                // Object is not found
                if (res == FR_NO_FILE) {
                    // If dot entry is not exist, stay there
                    if (FF_FS_RPATH && (ns & NS_DOT)) {
                        // Continue to follow if not last segment
                        if (!(ns & NS_LAST)) {
                            continue;
                        }
                        dp->fn[NSFLAG] = NS_NONAME;
                        res            = FR_OK;
                        // Could not find the object
                    }
                    else {
                        // Adjust error code if not last segment
                        if (!(ns & NS_LAST)) {
                            res = FR_NO_PATH;
                        }
                    }
                }
                break;
            }
            // Last segment matched. Function completed.
            if (ns & NS_LAST) {
                break;
            }
            // Get into the sub-directory

            // It is not a sub-directory and cannot follow
            if (!(dp->obj.attr & AM_DIR)) {
                res = FR_NO_PATH;
                break;
            }
#if FF_FS_EXFAT
            // Save containing directory information for next dir
            if (fs->fs_type == FS_EXFAT) {
                dp->obj.c_scl = dp->obj.sclust;
                dp->obj.c_size
                  = ((DWORD)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
                dp->obj.c_ofs = dp->blk_ofs;
                // Open next directory
                init_alloc_info(fs, &dp->obj);
            }
            else
#endif
            {
                // Open next directory
                dp->obj.sclust = ld_clust(fs, fs->win + dp->dptr % SS(fs));
            }
        }
    }

    return res;
}

//-----------------------------------------------------------------------

// Get logical drive number from path name

//-----------------------------------------------------------------------

// Returns logical drive number (-1:invalid drive number or null pointer)
static int get_ldnumber(
  // Pointer to pointer to the path name
  const TCHAR** path) {
    const TCHAR* tp;
    const TCHAR* tt;
    TCHAR        tc;
    int          i;
    int          vol = -1;
    // Find string volume ID
#if FF_STR_VOLUME_ID
    const char* sp;
    char        c;
#endif

    tt = tp = *path;
    // Invalid path name?
    if (!tp) {
        return vol;
    }
    // Find a colon in the path
    do {
        tc = *tt++;
    } while (!IsTerminator(tc) && tc != ':');

    // DOS/Windows style volume ID?
    if (tc == ':') {
        i = FF_VOLUMES;
        // Is there a numeric volume ID + colon?
        if (IsDigit(*tp) && tp + 2 == tt) {
            // Get the LD number
            i = (int)*tp - '0';
        }
        // Arbitrary string is enabled
#if FF_STR_VOLUME_ID == 1
        else {
            i = 0;
            do {
                // This string volume ID and path name
                sp = VolumeStr[i];
                tp = *path;
                // Compare the volume ID with path name
                do {
                    c  = *sp++;
                    tc = *tp++;
                    if (IsLower(c)) {
                        c -= 0x20;
                    }
                    if (IsLower(tc)) {
                        tc -= 0x20;
                    }
                } while (c && (TCHAR)c == tc);
                // Repeat for each id until pattern match
            } while ((c || tp != tt) && ++i < FF_VOLUMES);
        }
#endif
        // If a volume ID is found, get the drive number and strip it
        if (i < FF_VOLUMES) {
            // Drive number
            vol   = i;
            // Snip the drive prefix off
            *path = tt;
        }
        return vol;
    }
    // Unix style volume ID is enabled
#if FF_STR_VOLUME_ID == 2
    // Is there a volume ID?
    if (*tp == '/') {
        // Skip duplicated separator
        while (*(tp + 1) == '/') {
            tp++;
        }
        i = 0;
        do {
            // Path name and this string volume ID
            tt = tp;
            sp = VolumeStr[i];
            // Compare the volume ID with path name
            do {
                c  = *sp++;
                tc = *(++tt);
                if (IsLower(c)) {
                    c -= 0x20;
                }
                if (IsLower(tc)) {
                    tc -= 0x20;
                }
            } while (c && (TCHAR)c == tc);
            // Repeat for each ID until pattern match
        } while ((c || (tc != '/' && !IsTerminator(tc))) && ++i < FF_VOLUMES);
        // If a volume ID is found, get the drive number and strip it
        if (i < FF_VOLUMES) {
            // Drive number
            vol   = i;
            // Snip the drive prefix off
            *path = tt;
        }
        return vol;
    }
#endif
    // No drive prefix is found

#if FF_FS_RPATH != 0
    // Default drive is current drive
    vol = CurrVol;
#else
    // Default drive is 0
    vol = 0;
#endif
    // Return the default drive
    return vol;
}

//-----------------------------------------------------------------------

// GPT support functions

//-----------------------------------------------------------------------

#if FF_LBA64

// Calculate CRC32 in byte-by-byte

// Returns next CRC value
static DWORD crc32(
  // Current CRC value
  DWORD crc,
  // A byte to be processed
  BYTE  d) {
    BYTE b;

    for (b = 1; b; b <<= 1) {
        crc ^= (d & b) ? 1 : 0;
        crc  = (crc & 1) ? crc >> 1 ^ 0xEDB88320 : crc >> 1;
    }
    return crc;
}

// Check validity of GPT header

// 0:Invalid, 1:Valid
static int test_gpt_header(
  // Pointer to the GPT header
  const BYTE* gpth) {
    UINT  i;
    DWORD bcc, hlen;

    // Check signature and version (1.0)
    if (memcmp(gpth + GPTH_Sign,
               "EFI PART"
               "\0\0\1",
               12)) {
        return 0;
    }
    // Check header size
    hlen = ld_dword(gpth + GPTH_Size);
    if (hlen < 92 || hlen > FF_MIN_SS) {
        return 0;
    }
    // Check header BCC
    for (i = 0, bcc = 0xFFFFFFFF; i < hlen; i++) {
        bcc = crc32(bcc, i - GPTH_Bcc < 4 ? 0 : gpth[i]);
    }
    if (~bcc != ld_dword(gpth + GPTH_Bcc)) {
        return 0;
    }
    // Table entry size (must be SZ_GPTE bytes)
    if (ld_dword(gpth + GPTH_PteSize) != SZ_GPTE) {
        return 0;
    }
    // Table size (must be 128 entries or less)
    if (ld_dword(gpth + GPTH_PtNum) > 128) {
        return 0;
    }

    return 1;
}

#    if !FF_FS_READONLY && FF_USE_MKFS

// Generate random value

static DWORD make_rand(
  // Seed value
  DWORD seed,
  // Output buffer
  BYTE* buff,
  // Data length
  UINT  n) {
    UINT r;

    if (seed == 0) {
        seed = 1;
    }
    do {
        // Shift 8 bits the 32-bit LFSR
        for (r = 0; r < 8; r++) {
            seed = seed & 1 ? seed >> 1 ^ 0xA3000000 : seed >> 1;
        }
        *buff++ = (BYTE)seed;
    } while (--n);
    return seed;
}

#    endif
#endif

/*-----------------------------------------------------------------------*/
/* Load a sector and check if it is an FAT VBR                           */
/*-----------------------------------------------------------------------*/

// Check what the sector is

// 0:FAT/FAT32 VBR, 1:exFAT VBR, 2:Not FAT and valid BS, 3:Not FAT and invalid
// BS, 4:Disk error
static UINT check_fs(
  // Filesystem object
  FATFS* fs,
  // Sector to load and check if it is an FAT-VBR or not
  LBA_t  sect) {
    WORD w, sign;
    BYTE b;

    // Invaidate window
    fs->wflag   = 0;
    fs->winsect = (LBA_t)0 - 1;
    // Load the boot sector
    if (move_window(fs, sect) != FR_OK) {
        return 4;
    }
    sign = ld_word(fs->win + BS_55AA);
#if FF_FS_EXFAT
    // It is an exFAT VBR
    if (sign == 0xAA55
        && !memcmp(fs->win + BS_JmpBoot,
                   "\xEB\x76\x90"
                   "EXFAT   ",
                   11)) {
        return 1;
    }
#endif
    b = fs->win[BS_JmpBoot];
    // Valid JumpBoot code? (short jump, near jump or near call)
    if (b == 0xEB || b == 0xE9 || b == 0xE8) {
        if (sign == 0xAA55
            && !memcmp(fs->win + BS_FilSysType32, "FAT32   ", 8)) {
            // It is an FAT32 VBR
            return 0;
        }
        // FAT volumes formatted with early MS-DOS lack BS_55AA and
        // BS_FilSysType, so FAT VBR needs to be identified without them.

        w = ld_word(fs->win + BPB_BytsPerSec);
        b = fs->win[BPB_SecPerClus];
        // Properness of sector size (512-4096 and 2^n)
        if ((w & (w - 1)) == 0 && w >= FF_MIN_SS
            && w <= FF_MAX_SS
            // Properness of cluster size (2^n)
            && b != 0
            && (b & (b - 1)) == 0
            // Properness of reserved sectors (MNBZ)
            && ld_word(fs->win + BPB_RsvdSecCnt) != 0
            // Properness of FATs (1 or 2)
            && (UINT)fs->win[BPB_NumFATs] - 1 <= 1
            // Properness of root dir entries (MNBZ)
            && ld_word(fs->win + BPB_RootEntCnt) != 0
            // Properness of volume sectors (>=128)
            && (ld_word(fs->win + BPB_TotSec16) >= 128
                || ld_dword(fs->win + BPB_TotSec32) >= 0x10000)
            // Properness of FAT size (MNBZ)
            && ld_word(fs->win + BPB_FATSz16) != 0) {
            // It can be presumed an FAT VBR
            return 0;
        }
    }
    // Not an FAT VBR (valid or invalid BS)
    return sign == 0xAA55 ? 2 : 3;
}

// Find an FAT volume

// (It supports only generic partitioning rules, MBR, GPT and SFD)

// Returns BS status found in the hosting drive
static UINT find_volume(
  // Filesystem object
  FATFS* fs,
  // Partition to fined = 0:find as SFD and partitions, >0:forced partition
  // number
  UINT   part) {
    UINT  fmt, i;
    DWORD mbr_pt[4];

    // Load sector 0 and check if it is an FAT VBR as SFD format
    fmt = check_fs(fs, 0);
    // Returns if it is an FAT VBR as auto scan, not a BS or disk error
    if (fmt != 2 && (fmt >= 3 || part == 0)) {
        return fmt;
    }

    // Sector 0 is not an FAT VBR or forced partition number wants a partition

#if FF_LBA64
    // GPT protective MBR?
    if (fs->win[MBR_Table + PTE_System] == 0xEE) {
        DWORD n_ent, v_ent, ofs;
        QWORD pt_lba;

        // Load GPT header sector (next to MBR)
        if (move_window(fs, 1) != FR_OK) {
            return 4;
        }
        // Check if GPT header is valid
        if (!test_gpt_header(fs->win)) {
            return 3;
        }
        // Number of entries
        n_ent  = ld_dword(fs->win + GPTH_PtNum);
        // Table location
        pt_lba = ld_qword(fs->win + GPTH_PtOfs);
        // Find FAT partition
        for (v_ent = i = 0; i < n_ent; i++) {
            // PT sector
            if (move_window(fs, pt_lba + i * SZ_GPTE / SS(fs)) != FR_OK) {
                return 4;
            }
            // Offset in the sector
            ofs = i * SZ_GPTE % SS(fs);
            // MS basic data partition?
            if (!memcmp(fs->win + ofs + GPTE_PtGuid, GUID_MS_Basic, 16)) {
                v_ent++;
                // Load VBR and check status
                fmt = check_fs(fs, ld_qword(fs->win + ofs + GPTE_FstLba));
                // Auto search (valid FAT volume found first)
                if (part == 0 && fmt <= 1) {
                    return fmt;
                }
                // Forced partition order (regardless of it is valid or not)
                if (part != 0 && v_ent == part) {
                    return fmt;
                }
            }
        }
        // Not found
        return 3;
    }
#endif
    // MBR has 4 partitions max
    if (FF_MULTI_PARTITION && part > 4) {
        return 3;
    }
    // Load partition offset in the MBR
    for (i = 0; i < 4; i++) {
        mbr_pt[i] = ld_dword(fs->win + MBR_Table + i * SZ_PTE + PTE_StLba);
    }
    // Table index to find first
    i = part ? part - 1 : 0;
    // Find an FAT volume
    do {
        // Check if the partition is FAT
        fmt = mbr_pt[i] ? check_fs(fs, mbr_pt[i]) : 3;
    } while (part == 0 && fmt >= 2 && ++i < 4);
    return fmt;
}

//-----------------------------------------------------------------------

// Determine logical drive number and mount the volume if needed

//-----------------------------------------------------------------------

// FR_OK(0): successful, !=0: an error occurred
static FRESULT mount_volume(
  // Pointer to pointer to the path name (drive number)
  const TCHAR** path,
  // Pointer to pointer to the found filesystem object
  FATFS**       rfs,
  // Desiered access mode to check write protection
  BYTE          mode) {
    int     vol;
    FATFS*  fs;
    DSTATUS stat;
    LBA_t   bsect;
    DWORD   tsect, sysect, fasize, nclst, szbfat;
    WORD    nrsv;
    UINT    fmt;

    // Get logical drive number

    *rfs = 0;
    vol  = get_ldnumber(path);
    if (vol < 0) {
        return FR_INVALID_DRIVE;
    }

    // Check if the filesystem object is valid or not

    // Get pointer to the filesystem object
    fs = FatFs[vol];
    // Is the filesystem object available?
    if (!fs) {
        return FR_NOT_ENABLED;
    }
#if FF_FS_REENTRANT
    // Lock the volume, and system if needed
    if (!lock_volume(fs, 1)) {
        return FR_TIMEOUT;
    }
#endif
    // Return pointer to the filesystem object
    *rfs  = fs;

    // Desired access mode, write access or not
    mode &= (BYTE)~FA_READ;
    // If the volume has been mounted
    if (fs->fs_type != 0) {
        stat = disk_status(fs->pdrv);
        // and the physical drive is kept initialized
        if (!(stat & STA_NOINIT)) {
            // Check write protection if needed
            if (!FF_FS_READONLY && mode && (stat & STA_PROTECT)) {
                return FR_WRITE_PROTECTED;
            }
            // The filesystem object is already valid
            return FR_OK;
        }
    }

    // The filesystem object is not valid.

    // Following code attempts to mount the volume. (find an FAT volume, analyze
    // the BPB and initialize the filesystem object)

    // Invalidate the filesystem object
    fs->fs_type = 0;
    // Initialize the volume hosting physical drive
    stat        = disk_initialize(fs->pdrv);
    // Check if the initialization succeeded
    if (stat & STA_NOINIT) {
        // Failed to initialize due to no medium or hard error
        return FR_NOT_READY;
    }
    // Check disk write protection if needed
    if (!FF_FS_READONLY && mode && (stat & STA_PROTECT)) {
        return FR_WRITE_PROTECTED;
    }
    // Get sector size (multiple sector size cfg only)
#if FF_MAX_SS != FF_MIN_SS
    if (disk_ioctl(fs->pdrv, GET_SECTOR_SIZE, &SS(fs)) != RES_OK) {
        return FR_DISK_ERR;
    }
    if (SS(fs) > FF_MAX_SS || SS(fs) < FF_MIN_SS || (SS(fs) & (SS(fs) - 1))) {
        return FR_DISK_ERR;
    }
#endif

    // Find an FAT volume on the hosting drive

    fmt = find_volume(fs, LD2PT(vol));
    // An error occurred in the disk I/O layer
    if (fmt == 4) {
        return FR_DISK_ERR;
    }
    // No FAT volume is found
    if (fmt >= 2) {
        return FR_NO_FILESYSTEM;
    }
    // Volume offset in the hosting physical drive
    bsect = fs->winsect;

    // An FAT volume is found (bsect). Following code initializes the filesystem
    // object

#if FF_FS_EXFAT
    if (fmt == 1) {
        QWORD maxlba;
        DWORD so, cv, bcl, i;

        // Check zero filler
        for (i = BPB_ZeroedEx; i < BPB_ZeroedEx + 53 && fs->win[i] == 0; i++)
            ;
        if (i < BPB_ZeroedEx + 53) {
            return FR_NO_FILESYSTEM;
        }

        // Check exFAT version (must be version 1.0)
        if (ld_word(fs->win + BPB_FSVerEx) != 0x100) {
            return FR_NO_FILESYSTEM;
        }

        // (BPB_BytsPerSecEx must be equal to the physical sector size)
        if (1 << fs->win[BPB_BytsPerSecEx] != SS(fs)) {
            return FR_NO_FILESYSTEM;
        }

        // Last LBA of the volume + 1
        maxlba = ld_qword(fs->win + BPB_TotSecEx) + bsect;
        // (It cannot be accessed in 32-bit LBA)
        if (!FF_LBA64 && maxlba >= 0x100000000) {
            return FR_NO_FILESYSTEM;
        }

        // Number of sectors per FAT
        fs->fsize  = ld_dword(fs->win + BPB_FatSzEx);

        // Number of FATs
        fs->n_fats = fs->win[BPB_NumFATsEx];
        // (Supports only 1 FAT)
        if (fs->n_fats != 1) {
            return FR_NO_FILESYSTEM;
        }

        // Cluster size
        fs->csize = 1 << fs->win[BPB_SecPerClusEx];
        // (Must be 1..32768 sectors)
        if (fs->csize == 0) {
            return FR_NO_FILESYSTEM;
        }

        // Number of clusters
        nclst = ld_dword(fs->win + BPB_NumClusEx);
        // (Too many clusters)
        if (nclst > MAX_EXFAT) {
            return FR_NO_FILESYSTEM;
        }
        fs->n_fatent = nclst + 2;

        // Boundaries and Limits

        fs->volbase  = bsect;
        fs->database = bsect + ld_dword(fs->win + BPB_DataOfsEx);
        fs->fatbase  = bsect + ld_dword(fs->win + BPB_FatOfsEx);
        // (Volume size must not be smaller than the size required)
        if (maxlba < (QWORD)fs->database + nclst * fs->csize) {
            return FR_NO_FILESYSTEM;
        }
        fs->dirbase = ld_dword(fs->win + BPB_RootClusEx);

        // Get bitmap location and check if it is contiguous (implementation
        // assumption)

        so = i = 0;
        // Find the bitmap entry in the root directory (in only first cluster)
        for (;;) {
            if (i == 0) {
                // Not found?
                if (so >= fs->csize) {
                    return FR_NO_FILESYSTEM;
                }
                if (move_window(fs, clst2sect(fs, (DWORD)fs->dirbase) + so)
                    != FR_OK) {
                    return FR_DISK_ERR;
                }
                so++;
            }
            // Is it a bitmap entry?
            if (fs->win[i] == ET_BITMAP) {
                break;
            }
            // Next entry
            i = (i + SZDIRE) % SS(fs);
        }
        // Bitmap cluster
        bcl = ld_dword(fs->win + i + 20);
        // (Wrong cluster#)
        if (bcl < 2 || bcl >= fs->n_fatent) {
            return FR_NO_FILESYSTEM;
        }
        // Bitmap sector
        fs->bitbase = fs->database + fs->csize * (bcl - 2);
        // Check if bitmap is contiguous
        for (;;) {
            if (move_window(fs, fs->fatbase + bcl / (SS(fs) / 4)) != FR_OK) {
                return FR_DISK_ERR;
            }
            cv = ld_dword(fs->win + bcl % (SS(fs) / 4) * 4);
            // Last link?
            if (cv == 0xFFFFFFFF) {
                break;
            }
            // Fragmented bitmap?
            if (cv != ++bcl) {
                return FR_NO_FILESYSTEM;
            }
        }

#    if !FF_FS_READONLY
        // Initialize cluster allocation information
        fs->last_clst = fs->free_clst = 0xFFFFFFFF;
#    endif
        // FAT sub-type
        fmt = FS_EXFAT;
    }
    else
    // FF_FS_EXFAT
#endif
    {
        // (BPB_BytsPerSec must be equal to the physical sector size)
        if (ld_word(fs->win + BPB_BytsPerSec) != SS(fs)) {
            return FR_NO_FILESYSTEM;
        }

        // Number of sectors per FAT
        fasize = ld_word(fs->win + BPB_FATSz16);
        if (fasize == 0) {
            fasize = ld_dword(fs->win + BPB_FATSz32);
        }
        fs->fsize  = fasize;

        // Number of FATs
        fs->n_fats = fs->win[BPB_NumFATs];
        // (Must be 1 or 2)
        if (fs->n_fats != 1 && fs->n_fats != 2) {
            return FR_NO_FILESYSTEM;
        }
        // Number of sectors for FAT area
        fasize    *= fs->n_fats;

        // Cluster size
        fs->csize  = fs->win[BPB_SecPerClus];
        // (Must be power of 2)
        if (fs->csize == 0 || (fs->csize & (fs->csize - 1))) {
            return FR_NO_FILESYSTEM;
        }

        // Number of root directory entries
        fs->n_rootdir = ld_word(fs->win + BPB_RootEntCnt);
        // (Must be sector aligned)
        if (fs->n_rootdir % (SS(fs) / SZDIRE)) {
            return FR_NO_FILESYSTEM;
        }

        // Number of sectors on the volume
        tsect = ld_word(fs->win + BPB_TotSec16);
        if (tsect == 0) {
            tsect = ld_dword(fs->win + BPB_TotSec32);
        }

        // Number of reserved sectors
        nrsv = ld_word(fs->win + BPB_RsvdSecCnt);
        // (Must not be 0)
        if (nrsv == 0) {
            return FR_NO_FILESYSTEM;
        }

        // Determine the FAT sub type

        // RSV + FAT + DIR
        sysect = nrsv + fasize + fs->n_rootdir / (SS(fs) / SZDIRE);
        // (Invalid volume size)
        if (tsect < sysect) {
            return FR_NO_FILESYSTEM;
        }
        // Number of clusters
        nclst = (tsect - sysect) / fs->csize;
        // (Invalid volume size)
        if (nclst == 0) {
            return FR_NO_FILESYSTEM;
        }
        fmt = 0;
        if (nclst <= MAX_FAT32) {
            fmt = FS_FAT32;
        }
        if (nclst <= MAX_FAT16) {
            fmt = FS_FAT16;
        }
        if (nclst <= MAX_FAT12) {
            fmt = FS_FAT12;
        }
        if (fmt == 0) {
            return FR_NO_FILESYSTEM;
        }

        // Boundaries and Limits

        // Number of FAT entries
        fs->n_fatent = nclst + 2;
        // Volume start sector
        fs->volbase  = bsect;
        // FAT start sector
        fs->fatbase  = bsect + nrsv;
        // Data start sector
        fs->database = bsect + sysect;
        if (fmt == FS_FAT32) {
            // (Must be FAT32 revision 0.0)
            if (ld_word(fs->win + BPB_FSVer32) != 0) {
                return FR_NO_FILESYSTEM;
            }
            // (BPB_RootEntCnt must be 0)
            if (fs->n_rootdir != 0) {
                return FR_NO_FILESYSTEM;
            }
            // Root directory start cluster
            fs->dirbase = ld_dword(fs->win + BPB_RootClus32);
            // (Needed FAT size)
            szbfat      = fs->n_fatent * 4;
        }
        else {
            // (BPB_RootEntCnt must not be 0)
            if (fs->n_rootdir == 0) {
                return FR_NO_FILESYSTEM;
            }
            // Root directory start sector
            fs->dirbase = fs->fatbase + fasize;
            // (Needed FAT size)
            szbfat      = (fmt == FS_FAT16)
                          ? fs->n_fatent * 2
                          : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
        }
        // (BPB_FATSz must not be less than the size needed)
        if (fs->fsize < (szbfat + (SS(fs) - 1)) / SS(fs)) {
            return FR_NO_FILESYSTEM;
        }

#if !FF_FS_READONLY
        // Get FSInfo if available

        // Initialize cluster allocation information
        fs->last_clst = fs->free_clst = 0xFFFFFFFF;
        fs->fsi_flag                  = 0x80;
#    if (FF_FS_NOFSINFO & 3) != 3
        // Allow to update FSInfo only if BPB_FSInfo32 == 1
        if (fmt == FS_FAT32 && ld_word(fs->win + BPB_FSInfo32) == 1
            && move_window(fs, bsect + 1) == FR_OK) {
            fs->fsi_flag = 0;
            // Load FSInfo data if available
            if (ld_word(fs->win + BS_55AA) == 0xAA55
                && ld_dword(fs->win + FSI_LeadSig) == 0x41615252
                && ld_dword(fs->win + FSI_StrucSig) == 0x61417272) {
#        if (FF_FS_NOFSINFO & 1) == 0
                fs->free_clst = ld_dword(fs->win + FSI_Free_Count);
#        endif
#        if (FF_FS_NOFSINFO & 2) == 0
                fs->last_clst = ld_dword(fs->win + FSI_Nxt_Free);
#        endif
            }
        }
        // (FF_FS_NOFSINFO & 3) != 3
#    endif
// !FF_FS_READONLY
#endif
    }

    // FAT sub-type (the filesystem object gets valid)
    fs->fs_type = (BYTE)fmt;
    // Volume mount ID
    fs->id      = ++Fsid;
#if FF_USE_LFN == 1
    // Static LFN working buffer
    fs->lfnbuf = LfnBuf;
#    if FF_FS_EXFAT
    // Static directory block scratchpad buuffer
    fs->dirbuf = DirBuf;
#    endif
#endif
#if FF_FS_RPATH != 0
    // Initialize current directory
    fs->cdir = 0;
#endif
// Clear file lock semaphores
#if FF_FS_LOCK
    clear_share(fs);
#endif
    return FR_OK;
}

//-----------------------------------------------------------------------

// Check if the file/directory object is valid or not

//-----------------------------------------------------------------------

// Returns FR_OK or FR_INVALID_OBJECT
static FRESULT validate(
  // Pointer to the FFOBJID, the 1st member in the FIL/DIR structure, to check
  // validity
  FFOBJID* obj,
  // Pointer to pointer to the owner filesystem object to return
  FATFS**  rfs) {
    FRESULT res = FR_INVALID_OBJECT;

    // Test if the object is valid
    if (obj && obj->fs && obj->fs->fs_type && obj->id == obj->fs->id) {
#if FF_FS_REENTRANT
        // Take a grant to access the volume
        if (lock_volume(obj->fs, 0)) {
            // Test if the hosting phsical drive is kept initialized
            if (!(disk_status(obj->fs->pdrv) & STA_NOINIT)) {
                res = FR_OK;
            }
            else {
                // Invalidated volume, abort to access
                unlock_volume(obj->fs, FR_OK);
            }
            // Could not take
        }
        else {
            res = FR_TIMEOUT;
        }
#else
        // Test if the hosting phsical drive is kept initialized
        if (!(disk_status(obj->fs->pdrv) & STA_NOINIT)) {
            res = FR_OK;
        }
#endif
    }
    // Return corresponding filesystem object if it is valid
    *rfs = (res == FR_OK) ? obj->fs : 0;
    return res;
}

//-------------------------------------------------------------------------

// Public Functions (FatFs API)

// ----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------

// Mount/Unmount a Logical Drive

//-----------------------------------------------------------------------

FRESULT f_mount(
  //
  FATFS* fs, const TCHAR* path, BYTE opt) {
    FATFS*       cfs;
    int          vol;
    FRESULT      res;
    const TCHAR* rp = path;

    // Get volume ID (logical drive number)

    vol             = get_ldnumber(&rp);
    if (vol < 0) {
        return FR_INVALID_DRIVE;
    }
    // Pointer to the filesystem object of the volume
    cfs = FatFs[vol];

    // Unregister current filesystem object if regsitered
    if (cfs) {
        FatFs[vol] = 0;
#if FF_FS_LOCK
        clear_share(cfs);
#endif
// Discard mutex of the current volume
#if FF_FS_REENTRANT
        ff_mutex_delete(vol);
#endif
        // Invalidate the filesystem object to be unregistered
        cfs->fs_type = 0;
    }

    // Register new filesystem object
    if (fs) {
        // Volume hosting physical drive
        fs->pdrv = LD2PD(vol);
        // Create a volume mutex
#if FF_FS_REENTRANT
        // Owner volume ID
        fs->ldrv = (BYTE)vol;
        if (!ff_mutex_create(vol)) {
            return FR_INT_ERR;
        }
#    if FF_FS_LOCK
        // Create a system mutex if needed
        if (SysLock == 0) {
            if (!ff_mutex_create(FF_VOLUMES)) {
                ff_mutex_delete(vol);
                return FR_INT_ERR;
            }
            // System mutex is ready
            SysLock = 1;
        }
#    endif
#endif
        // Invalidate the new filesystem object
        fs->fs_type = 0;
        // Register new fs object
        FatFs[vol]  = fs;
    }

    // Do not mount now, it will be mounted in subsequent file functions
    if (opt == 0) {
        return FR_OK;
    }

    // Force mounted the volume
    res = mount_volume(&path, &fs, 0);
    LEAVE_FF(fs, res);
}

FRESULT f_open(FIL* fp, const TCHAR* path, BYTE mode) {
    FRESULT res;
    DIR     dj;
    FATFS*  fs;
#if !FF_FS_READONLY
    DWORD   cl, bcs, clst, tm;
    LBA_t   sc;
    FSIZE_t ofs;
#endif
    DEF_NAMBUF

    if (!fp) {
        return FR_INVALID_OBJECT;
    }

    // Get logical drive number

    mode &= FF_FS_READONLY
            ? FA_READ
            : FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW
                | FA_OPEN_ALWAYS | FA_OPEN_APPEND;
    res   = mount_volume(&path, &fs, mode);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the file path
        res = follow_path(&dj, path);
        // Read/Write configuration
#if !FF_FS_READONLY
        if (res == FR_OK) {
            // Origin directory itself?
            if (dj.fn[NSFLAG] & NS_NONAME) {
                res = FR_INVALID_NAME;
            }
#    if FF_FS_LOCK
            else {
                // Check if the file can be used
                res = chk_share(&dj, (mode & ~FA_READ) ? 1 : 0);
            }
#    endif
        }
        // Create or Open a file

        if (mode & (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW)) {
            // No file, create new
            if (res != FR_OK) {
                // There is no file to open, create a new entry
                if (res == FR_NO_FILE) {
#    if FF_FS_LOCK
                    res = enq_share() ? dir_register(&dj)
                                      : FR_TOO_MANY_OPEN_FILES;
#    else
                    res = dir_register(&dj);
#    endif
                }
                // File is created
                mode |= FA_CREATE_ALWAYS;
            }
            // Any object with the same name is already existing
            else {
                // Cannot overwrite it (R/O or DIR)
                if (dj.obj.attr & (AM_RDO | AM_DIR)) {
                    res = FR_DENIED;
                }
                else {
                    // Cannot create as new file
                    if (mode & FA_CREATE_NEW) {
                        res = FR_EXIST;
                    }
                }
            }
            // Truncate the file if overwrite mode
            if (res == FR_OK && (mode & FA_CREATE_ALWAYS)) {
#    if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    // Get current allocation info

                    fp->obj.fs = fs;
                    init_alloc_info(fs, &fp->obj);
                    // Set directory entry block initial state

                    // Clear 85 entry except for NumSec
                    memset(fs->dirbuf + 2, 0, 30);
                    // Clear C0 entry except for NumName and NameHash
                    memset(fs->dirbuf + 38, 0, 26);
                    fs->dirbuf[XDIR_Attr] = AM_ARC;
                    st_dword(fs->dirbuf + XDIR_CrtTime, GET_FATTIME());
                    fs->dirbuf[XDIR_GenFlags] = 1;
                    res                       = store_xdir(&dj);
                    // Remove the cluster chain if exist
                    if (res == FR_OK && fp->obj.sclust != 0) {
                        res = remove_chain(&fp->obj, fp->obj.sclust, 0);
                        // Reuse the cluster hole
                        fs->last_clst = fp->obj.sclust - 1;
                    }
                }
                else
#    endif
                {
                    // Set directory entry initial state

                    // Set created time
                    tm = GET_FATTIME();
                    st_dword(dj.dir + DIR_CrtTime, tm);
                    st_dword(dj.dir + DIR_ModTime, tm);
                    // Get current cluster chain
                    cl               = ld_clust(fs, dj.dir);
                    // Reset attribute
                    dj.dir[DIR_Attr] = AM_ARC;
                    // Reset file allocation info
                    st_clust(fs, dj.dir, 0);
                    st_dword(dj.dir + DIR_FileSize, 0);
                    fs->wflag = 1;
                    // Remove the cluster chain if exist
                    if (cl != 0) {
                        sc  = fs->winsect;
                        res = remove_chain(&dj.obj, cl, 0);
                        if (res == FR_OK) {
                            res           = move_window(fs, sc);
                            // Reuse the cluster hole
                            fs->last_clst = cl - 1;
                        }
                    }
                }
            }
        }
        // Open an existing file
        else {
            // Is the object exsiting?
            if (res == FR_OK) {
                // File open against a directory
                if (dj.obj.attr & AM_DIR) {
                    res = FR_NO_FILE;
                }
                else {
                    // Write mode open against R/O file
                    if ((mode & FA_WRITE) && (dj.obj.attr & AM_RDO)) {
                        res = FR_DENIED;
                    }
                }
            }
        }
        if (res == FR_OK) {
            // Set file change flag if created or overwritten
            if (mode & FA_CREATE_ALWAYS) {
                mode |= FA_MODIFIED;
            }
            // Pointer to the directory entry
            fp->dir_sect = fs->winsect;
            fp->dir_ptr  = dj.dir;
#    if FF_FS_LOCK
            // Lock the file for this session
            fp->obj.lockid = inc_share(&dj, (mode & ~FA_READ) ? 1 : 0);
            if (fp->obj.lockid == 0) {
                res = FR_INT_ERR;
            }
#    endif
        }
        // R/O configuration
#else
        if (res == FR_OK) {
            // Is it origin directory itself?
            if (dj.fn[NSFLAG] & NS_NONAME) {
                res = FR_INVALID_NAME;
            }
            else {
                // Is it a directory?
                if (dj.obj.attr & AM_DIR) {
                    res = FR_NO_FILE;
                }
            }
        }
#endif

        if (res == FR_OK) {
#if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                // Get containing directory info
                fp->obj.c_scl = dj.obj.sclust;
                fp->obj.c_size
                  = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                fp->obj.c_ofs = dj.blk_ofs;
                init_alloc_info(fs, &fp->obj);
            }
            else
#endif
            {
                // Get object allocation info
                fp->obj.sclust  = ld_clust(fs, dj.dir);
                fp->obj.objsize = ld_dword(dj.dir + DIR_FileSize);
            }
#if FF_USE_FASTSEEK
            // Disable fast seek mode
            fp->cltbl = 0;
#endif
            // Validate the file object
            fp->obj.fs = fs;
            fp->obj.id = fs->id;
            // Set file access mode
            fp->flag   = mode;
            // Clear error flag
            fp->err    = 0;
            // Invalidate current data sector
            fp->sect   = 0;
            // Set file pointer top of the file
            fp->fptr   = 0;
#if !FF_FS_READONLY
#    if !FF_FS_TINY
            // Clear sector buffer
            memset(fp->buf, 0, sizeof fp->buf);
#    endif
            // Seek to end of file if FA_OPEN_APPEND is specified
            if ((mode & FA_SEEKEND) && fp->obj.objsize > 0) {
                // Offset to seek
                fp->fptr = fp->obj.objsize;
                // Cluster size in byte
                bcs      = (DWORD)fs->csize * SS(fs);
                // Follow the cluster chain
                clst     = fp->obj.sclust;
                for (ofs  = fp->obj.objsize; res == FR_OK && ofs > bcs;
                     ofs -= bcs) {
                    clst = get_fat(&fp->obj, clst);
                    if (clst <= 1) {
                        res = FR_INT_ERR;
                    }
                    if (clst == 0xFFFFFFFF) {
                        res = FR_DISK_ERR;
                    }
                }
                fp->clust = clst;
                // Fill sector buffer if not on the sector boundary
                if (res == FR_OK && ofs % SS(fs)) {
                    sc = clst2sect(fs, clst);
                    if (sc == 0) {
                        res = FR_INT_ERR;
                    }
                    else {
                        fp->sect = sc + (DWORD)(ofs / SS(fs));
#    if !FF_FS_TINY
                        if (disk_read(fs->pdrv, fp->buf, fp->sect, 1)
                            != RES_OK) {
                            res = FR_DISK_ERR;
                        }
#    endif
                    }
                }
#    if FF_FS_LOCK
                // Decrement file open counter if seek failed
                if (res != FR_OK) {
                    dec_share(fp->obj.lockid);
                }
#    endif
            }
#endif
        }

        FREE_NAMBUF();
    }

    // Invalidate file object on error
    if (res != FR_OK) {
        fp->obj.fs = 0;
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Read File

//-----------------------------------------------------------------------

FRESULT f_read(FIL* fp, void* buff, UINT btr, UINT* br) {
    FRESULT res;
    FATFS*  fs;
    DWORD   clst;
    LBA_t   sect;
    FSIZE_t remain;
    UINT    rcnt, cc, csect;
    BYTE*   rbuff = (BYTE*)buff;

    // Clear read byte counter
    *br           = 0;
    // Check validity of the file object
    res           = validate(&fp->obj, &fs);
    // Check validity
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) {
        LEAVE_FF(fs, res);
    }
    // Check access mode
    if (!(fp->flag & FA_READ)) {
        LEAVE_FF(fs, FR_DENIED);
    }
    remain = fp->obj.objsize - fp->fptr;
    // Truncate btr by remaining bytes
    if (btr > remain) {
        btr = (UINT)remain;
    }

    // Repeat until btr bytes read
    for (; btr > 0; btr -= rcnt, *br += rcnt, rbuff += rcnt, fp->fptr += rcnt) {
        // On the sector boundary?
        if (fp->fptr % SS(fs) == 0) {
            // Sector offset in the cluster
            csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));
            // On the cluster boundary?
            if (csect == 0) {
                // On the top of the file?
                if (fp->fptr == 0) {
                    // Follow cluster chain from the origin
                    clst = fp->obj.sclust;
                    // Middle or end of the file
                }
                else {
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        // Get cluster# from the CLMT
                        clst = clmt_clust(fp, fp->fptr);
                    }
                    else
#endif
                    {
                        // Follow cluster chain on the FAT
                        clst = get_fat(&fp->obj, fp->clust);
                    }
                }
                if (clst < 2) {
                    ABORT(fs, FR_INT_ERR);
                }
                if (clst == 0xFFFFFFFF) {
                    ABORT(fs, FR_DISK_ERR);
                }
                // Update current cluster
                fp->clust = clst;
            }
            // Get current sector
            sect = clst2sect(fs, fp->clust);
            if (sect == 0) {
                ABORT(fs, FR_INT_ERR);
            }
            sect += csect;
            // When remaining bytes >= sector size,
            cc    = btr / SS(fs);
            // Read maximum contiguous sectors directly
            if (cc > 0) {
                // Clip at cluster boundary
                if (csect + cc > fs->csize) {
                    cc = fs->csize - csect;
                }
                if (disk_read(fs->pdrv, rbuff, sect, cc) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
                // Replace one of the read sectors with cached data if it
                // contains a dirty sector
#if !FF_FS_READONLY && FF_FS_MINIMIZE <= 2
#    if FF_FS_TINY
                if (fs->wflag && fs->winsect - sect < cc) {
                    memcpy(rbuff + ((fs->winsect - sect) * SS(fs)), fs->win,
                           SS(fs));
                }
#    else
                if ((fp->flag & FA_DIRTY) && fp->sect - sect < cc) {
                    memcpy(rbuff + ((fp->sect - sect) * SS(fs)), fp->buf,
                           SS(fs));
                }
#    endif
#endif
                // Number of bytes transferred
                rcnt = SS(fs) * cc;
                continue;
            }
#if !FF_FS_TINY
            // Load data sector if not in cache
            if (fp->sect != sect) {
#    if !FF_FS_READONLY
                // Write-back dirty sector cache
                if (fp->flag & FA_DIRTY) {
                    if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                        ABORT(fs, FR_DISK_ERR);
                    }
                    fp->flag &= (BYTE)~FA_DIRTY;
                }
#    endif
                // Fill sector cache
                if (disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
            }
#endif
            fp->sect = sect;
        }
        // Number of bytes remains in the sector
        rcnt = SS(fs) - (UINT)fp->fptr % SS(fs);
        // Clip it by btr if needed
        if (rcnt > btr) {
            rcnt = btr;
        }
#if FF_FS_TINY
        // Move sector window
        if (move_window(fs, fp->sect) != FR_OK) {
            ABORT(fs, FR_DISK_ERR);
        }
        // Extract partial sector
        memcpy(rbuff, fs->win + fp->fptr % SS(fs), rcnt);
#else
        // Extract partial sector
        memcpy(rbuff, fp->buf + fp->fptr % SS(fs), rcnt);
#endif
    }

    LEAVE_FF(fs, FR_OK);
}

#if !FF_FS_READONLY
//-----------------------------------------------------------------------

// Write File

//-----------------------------------------------------------------------

FRESULT f_write(FIL* fp, const void* buff, UINT btw, UINT* bw) {
    FRESULT     res;
    FATFS*      fs;
    DWORD       clst;
    LBA_t       sect;
    UINT        wcnt, cc, csect;
    const BYTE* wbuff = (const BYTE*)buff;

    // Clear write byte counter
    *bw               = 0;
    // Check validity of the file object
    res               = validate(&fp->obj, &fs);
    // Check validity
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) {
        LEAVE_FF(fs, res);
    }
    // Check access mode
    if (!(fp->flag & FA_WRITE)) {
        LEAVE_FF(fs, FR_DENIED);
    }

    // Check fptr wrap-around (file size cannot reach 4 GiB at FAT volume)

    if ((!FF_FS_EXFAT || fs->fs_type != FS_EXFAT)
        && (DWORD)(fp->fptr + btw) < (DWORD)fp->fptr) {
        btw = (UINT)(0xFFFFFFFF - (DWORD)fp->fptr);
    }

    // Repeat until all data written
    for (; btw > 0; btw -= wcnt, *bw += wcnt, wbuff += wcnt, fp->fptr += wcnt,
                    fp->obj.objsize = (fp->fptr > fp->obj.objsize)
                                      ? fp->fptr
                                      : fp->obj.objsize) {
        // On the sector boundary?
        if (fp->fptr % SS(fs) == 0) {
            // Sector offset in the cluster
            csect = (UINT)(fp->fptr / SS(fs)) & (fs->csize - 1);
            // On the cluster boundary?
            if (csect == 0) {
                // On the top of the file?
                if (fp->fptr == 0) {
                    // Follow from the origin
                    clst = fp->obj.sclust;
                    // If no cluster is allocated,
                    if (clst == 0) {
                        // create a new cluster chain
                        clst = create_chain(&fp->obj, 0);
                    }
                    // On the middle or end of the file
                }
                else {
#    if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        // Get cluster# from the CLMT
                        clst = clmt_clust(fp, fp->fptr);
                    }
                    else
#    endif
                    {
                        // Follow or stretch cluster chain on the FAT
                        clst = create_chain(&fp->obj, fp->clust);
                    }
                }
                // Could not allocate a new cluster (disk full)
                if (clst == 0) {
                    break;
                }
                if (clst == 1) {
                    ABORT(fs, FR_INT_ERR);
                }
                if (clst == 0xFFFFFFFF) {
                    ABORT(fs, FR_DISK_ERR);
                }
                // Update current cluster
                fp->clust = clst;
                // Set start cluster if the first write
                if (fp->obj.sclust == 0) {
                    fp->obj.sclust = clst;
                }
            }
#    if FF_FS_TINY
            // Write-back sector cache
            if (fs->winsect == fp->sect && sync_window(fs) != FR_OK) {
                ABORT(fs, FR_DISK_ERR);
            }
#    else
            // Write-back sector cache
            if (fp->flag & FA_DIRTY) {
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#    endif
            // Get current sector
            sect = clst2sect(fs, fp->clust);
            if (sect == 0) {
                ABORT(fs, FR_INT_ERR);
            }
            sect += csect;
            // When remaining bytes >= sector size,
            cc    = btw / SS(fs);
            // Write maximum contiguous sectors directly
            if (cc > 0) {
                // Clip at cluster boundary
                if (csect + cc > fs->csize) {
                    cc = fs->csize - csect;
                }
                if (disk_write(fs->pdrv, wbuff, sect, cc) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
#    if FF_FS_MINIMIZE <= 2
#        if FF_FS_TINY
                // Refill sector cache if it gets invalidated by the direct
                // write
                if (fs->winsect - sect < cc) {
                    memcpy(fs->win, wbuff + ((fs->winsect - sect) * SS(fs)),
                           SS(fs));
                    fs->wflag = 0;
                }
#        else
                // Refill sector cache if it gets invalidated by the direct
                // write
                if (fp->sect - sect < cc) {
                    memcpy(fp->buf, wbuff + ((fp->sect - sect) * SS(fs)),
                           SS(fs));
                    fp->flag &= (BYTE)~FA_DIRTY;
                }
#        endif
#    endif
                // Number of bytes transferred
                wcnt = SS(fs) * cc;
                continue;
            }
#    if FF_FS_TINY
            // Avoid silly cache filling on the growing edge
            if (fp->fptr >= fp->obj.objsize) {
                if (sync_window(fs) != FR_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
                fs->winsect = sect;
            }
#    else
            // Fill sector cache with file data
            if (fp->sect != sect && fp->fptr < fp->obj.objsize
                && disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) {
                ABORT(fs, FR_DISK_ERR);
            }
#    endif
            fp->sect = sect;
        }
        // Number of bytes remains in the sector
        wcnt = SS(fs) - (UINT)fp->fptr % SS(fs);
        // Clip it by btw if needed
        if (wcnt > btw) {
            wcnt = btw;
        }
#    if FF_FS_TINY
        // Move sector window
        if (move_window(fs, fp->sect) != FR_OK) {
            ABORT(fs, FR_DISK_ERR);
        }
        // Fit data to the sector
        memcpy(fs->win + fp->fptr % SS(fs), wbuff, wcnt);
        fs->wflag = 1;
#    else
        // Fit data to the sector
        memcpy(fp->buf + fp->fptr % SS(fs), wbuff, wcnt);
        fp->flag |= FA_DIRTY;
#    endif
    }

    // Set file change flag
    fp->flag |= FA_MODIFIED;

    LEAVE_FF(fs, FR_OK);
}

//-----------------------------------------------------------------------

// Synchronize the File

//-----------------------------------------------------------------------

FRESULT f_sync(FIL* fp) {
    FRESULT res;
    FATFS*  fs;
    DWORD   tm;
    BYTE*   dir;

    // Check validity of the file object
    res = validate(&fp->obj, &fs);
    if (res == FR_OK) {
        // Is there any change to the file?
        if (fp->flag & FA_MODIFIED) {
#    if !FF_FS_TINY
            // Write-back cached data if needed
            if (fp->flag & FA_DIRTY) {
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                    LEAVE_FF(fs, FR_DISK_ERR);
                }
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#    endif
            // Update the directory entry

            // Modified time
            tm = GET_FATTIME();
#    if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                // Fill first fragment on the FAT if needed
                res = fill_first_frag(&fp->obj);
                if (res == FR_OK) {
                    // Fill last fragment on the FAT if needed
                    res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);
                }
                if (res == FR_OK) {
                    DIR dj;
                    DEF_NAMBUF

                    INIT_NAMBUF(fs);
                    // Load directory entry block
                    res = load_obj_xdir(&dj, &fp->obj);
                    if (res == FR_OK) {
                        // Set archive attribute to indicate that the file has
                        // been changed
                        fs->dirbuf[XDIR_Attr]     |= AM_ARC;
                        // Update file allocation information
                        fs->dirbuf[XDIR_GenFlags]  = fp->obj.stat | 1;
                        // Update start cluster
                        st_dword(fs->dirbuf + XDIR_FstClus, fp->obj.sclust);
                        // Update file size
                        st_qword(fs->dirbuf + XDIR_FileSize, fp->obj.objsize);
                        // (FatFs does not support Valid File Size feature)
                        st_qword(fs->dirbuf + XDIR_ValidFileSize,
                                 fp->obj.objsize);
                        // Update modified time
                        st_dword(fs->dirbuf + XDIR_ModTime, tm);
                        fs->dirbuf[XDIR_ModTime10] = 0;
                        st_dword(fs->dirbuf + XDIR_AccTime, 0);
                        // Restore it to the directory
                        res = store_xdir(&dj);
                        if (res == FR_OK) {
                            res       = sync_fs(fs);
                            fp->flag &= (BYTE)~FA_MODIFIED;
                        }
                    }
                    FREE_NAMBUF();
                }
            }
            else
#    endif
            {
                res = move_window(fs, fp->dir_sect);
                if (res == FR_OK) {
                    dir            = fp->dir_ptr;
                    // Set archive attribute to indicate that the file has been
                    // changed
                    dir[DIR_Attr] |= AM_ARC;
                    // Update file allocation information
                    st_clust(fp->obj.fs, dir, fp->obj.sclust);
                    // Update file size
                    st_dword(dir + DIR_FileSize, (DWORD)fp->obj.objsize);
                    // Update modified time
                    st_dword(dir + DIR_ModTime, tm);
                    st_word(dir + DIR_LstAccDate, 0);
                    fs->wflag  = 1;
                    // Restore it to the directory
                    res        = sync_fs(fs);
                    fp->flag  &= (BYTE)~FA_MODIFIED;
                }
            }
        }
    }

    LEAVE_FF(fs, res);
}

// !FF_FS_READONLY
#endif

//-----------------------------------------------------------------------

// Close File

//-----------------------------------------------------------------------

FRESULT f_close(FIL* fp) {
    FRESULT res;
    FATFS*  fs;

#if !FF_FS_READONLY
    // Flush cached data
    res = f_sync(fp);
    if (res == FR_OK)
#endif
    {
        // Lock volume
        res = validate(&fp->obj, &fs);
        if (res == FR_OK) {
#if FF_FS_LOCK
            // Decrement file open counter
            res = dec_share(fp->obj.lockid);
            // Invalidate file object
            if (res == FR_OK) {
                fp->obj.fs = 0;
            }
#else
            // Invalidate file object
            fp->obj.fs = 0;
#endif
#if FF_FS_REENTRANT
            // Unlock volume
            unlock_volume(fs, FR_OK);
#endif
        }
    }
    return res;
}

#if FF_FS_RPATH >= 1
//-----------------------------------------------------------------------

// Change Current Directory or Current Drive, Get Current Directory

//-----------------------------------------------------------------------

FRESULT f_chdrive(const TCHAR* path) {
    int vol;

    // Get logical drive number

    vol = get_ldnumber(&path);
    if (vol < 0) {
        return FR_INVALID_DRIVE;
    }
    // Set it as current volume
    CurrVol = (BYTE)vol;

    return FR_OK;
}

FRESULT f_chdir(const TCHAR* path) {
#    if FF_STR_VOLUME_ID == 2
    UINT i;
#    endif
    FRESULT res;
    DIR     dj;
    FATFS*  fs;
    DEF_NAMBUF

    // Get logical drive

    res = mount_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the path
        res = follow_path(&dj, path);
        // Follow completed
        if (res == FR_OK) {
            // Is it the start directory itself?
            if (dj.fn[NSFLAG] & NS_NONAME) {
                fs->cdir = dj.obj.sclust;
#    if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    fs->cdc_scl  = dj.obj.c_scl;
                    fs->cdc_size = dj.obj.c_size;
                    fs->cdc_ofs  = dj.obj.c_ofs;
                }
#    endif
            }
            else {
                // It is a sub-directory
                if (dj.obj.attr & AM_DIR) {
#    if FF_FS_EXFAT
                    if (fs->fs_type == FS_EXFAT) {
                        // Sub-directory cluster
                        fs->cdir    = ld_dword(fs->dirbuf + XDIR_FstClus);
                        // Save containing directory information
                        fs->cdc_scl = dj.obj.sclust;
                        fs->cdc_size
                          = ((DWORD)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                        fs->cdc_ofs = dj.blk_ofs;
                    }
                    else
#    endif
                    {
                        // Sub-directory cluster
                        fs->cdir = ld_clust(fs, dj.dir);
                    }
                }
                else {
                    // Reached but a file
                    res = FR_NO_PATH;
                }
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE) {
            res = FR_NO_PATH;
        }
        // Also current drive is changed if in Unix style volume ID
#    if FF_STR_VOLUME_ID == 2
        if (res == FR_OK) {
            // Set current drive
            for (i = FF_VOLUMES - 1; i && fs != FatFs[i]; i--)
                ;
            CurrVol = (BYTE)i;
        }
#    endif
    }

    LEAVE_FF(fs, res);
}

#    if FF_FS_RPATH >= 2
FRESULT f_getcwd(TCHAR* buff, UINT len) {
    FRESULT res;
    DIR     dj;
    FATFS*  fs;
    UINT    i, n;
    DWORD   ccl;
    TCHAR*  tp = buff;
#        if FF_VOLUMES >= 2
    UINT vl;
#            if FF_STR_VOLUME_ID
    const char* vp;
#            endif
#        endif
    FILINFO fno;
    DEF_NAMBUF

    // Get logical drive

    // Set null string to get current volume
    buff[0] = 0;
    // Get current volume
    res     = mount_volume((const TCHAR**)&buff, &fs, 0);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);

        // Follow parent directories and create the path

        // Bottom of buffer (directory stack base)
        i = len;
        // (Cannot do getcwd on exFAT and returns root path)
        if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
            // Start to follow upper directory from current directory
            dj.obj.sclust = fs->cdir;
            // Repeat while current directory is a sub-directory
            while ((ccl = dj.obj.sclust) != 0) {
                // Get parent directory
                res = dir_sdi(&dj, 1 * SZDIRE);
                if (res != FR_OK) {
                    break;
                }
                res = move_window(fs, dj.sect);
                if (res != FR_OK) {
                    break;
                }
                // Goto parent directory
                dj.obj.sclust = ld_clust(fs, dj.dir);
                res           = dir_sdi(&dj, 0);
                if (res != FR_OK) {
                    break;
                }
                // Find the entry links to the child directory
                do {
                    res = DIR_READ_FILE(&dj);
                    if (res != FR_OK) {
                        break;
                    }
                    // Found the entry
                    if (ccl == ld_clust(fs, dj.dir)) {
                        break;
                    }
                    res = dir_next(&dj, 0);
                } while (res == FR_OK);
                // It cannot be 'not found'.
                if (res == FR_NO_FILE) {
                    res = FR_INT_ERR;
                }
                if (res != FR_OK) {
                    break;
                }
                // Get the directory name and push it to the buffer
                get_fileinfo(&dj, &fno);
                // Name length
                for (n = 0; fno.fname[n]; n++)
                    ;
                // Insufficient space to store the path name?
                if (i < n + 1) {
                    res = FR_NOT_ENOUGH_CORE;
                    break;
                }
                // Stack the name
                while (n) {
                    buff[--i] = fno.fname[--n];
                }
                buff[--i] = '/';
            }
        }
        if (res == FR_OK) {
            // Is it the root-directory?
            if (i == len) {
                buff[--i] = '/';
            }
            // Put drive prefix
#        if FF_VOLUMES >= 2
            vl = 0;
            // String volume ID
#            if FF_STR_VOLUME_ID >= 1
            for (n = 0, vp = (const char*)VolumeStr[CurrVol]; vp[n]; n++)
                ;
            if (i >= n + 2) {
                if (FF_STR_VOLUME_ID == 2) {
                    *tp++ = (TCHAR)'/';
                }
                for (vl = 0; vl < n; *tp++ = (TCHAR)vp[vl], vl++)
                    ;
                if (FF_STR_VOLUME_ID == 1) {
                    *tp++ = (TCHAR)':';
                }
                vl++;
            }
            // Numeric volume ID
#            else
            if (i >= 3) {
                *tp++ = (TCHAR)'0' + CurrVol;
                *tp++ = (TCHAR)':';
                vl    = 2;
            }
#            endif
            if (vl == 0) {
                res = FR_NOT_ENOUGH_CORE;
            }
#        endif
            // Add current directory path

            if (res == FR_OK) {
                // Copy stacked path string
                do {
                    *tp++ = buff[i++];
                } while (i < len);
            }
        }
        FREE_NAMBUF();
    }

    *tp = 0;
    LEAVE_FF(fs, res);
}

// FF_FS_RPATH >= 2
#    endif
// FF_FS_RPATH >= 1
#endif

#if FF_FS_MINIMIZE <= 2
//-----------------------------------------------------------------------

// Seek File Read/Write Pointer

//-----------------------------------------------------------------------

FRESULT f_lseek(FIL* fp, FSIZE_t ofs) {
    FRESULT res;
    FATFS*  fs;
    DWORD   clst, bcs;
    LBA_t   nsect;
    FSIZE_t ifptr;
#    if FF_USE_FASTSEEK
    DWORD  cl, pcl, ncl, tcl, tlen, ulen;
    DWORD* tbl;
    LBA_t  dsc;
#    endif

    // Check validity of the file object
    res = validate(&fp->obj, &fs);
    if (res == FR_OK) {
        res = (FRESULT)fp->err;
    }
#    if FF_FS_EXFAT && !FF_FS_READONLY
    if (res == FR_OK && fs->fs_type == FS_EXFAT) {
        // Fill last fragment on the FAT if needed
        res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF);
    }
#    endif
    if (res != FR_OK) {
        LEAVE_FF(fs, res);
    }

#    if FF_USE_FASTSEEK
    // Fast seek
    if (fp->cltbl) {
        // Create CLMT
        if (ofs == CREATE_LINKMAP) {
            tbl  = fp->cltbl;
            // Given table size and required table size
            tlen = *tbl++;
            ulen = 2;
            // Origin of the chain
            cl   = fp->obj.sclust;
            if (cl != 0) {
                do {
                    // Get a fragment

                    // Top, length and used items
                    tcl   = cl;
                    ncl   = 0;
                    ulen += 2;
                    do {
                        pcl = cl;
                        ncl++;
                        cl = get_fat(&fp->obj, cl);
                        if (cl <= 1) {
                            ABORT(fs, FR_INT_ERR);
                        }
                        if (cl == 0xFFFFFFFF) {
                            ABORT(fs, FR_DISK_ERR);
                        }
                    } while (cl == pcl + 1);
                    // Store the length and top of the fragment
                    if (ulen <= tlen) {
                        *tbl++ = ncl;
                        *tbl++ = tcl;
                    }
                    // Repeat until end of chain
                } while (cl < fs->n_fatent);
            }
            // Number of items used
            *fp->cltbl = ulen;
            if (ulen <= tlen) {
                // Terminate table
                *tbl = 0;
            }
            else {
                // Given table size is smaller than required
                res = FR_NOT_ENOUGH_CORE;
            }
            // Fast seek
        }
        else {
            // Clip offset at the file size
            if (ofs > fp->obj.objsize) {
                ofs = fp->obj.objsize;
            }
            // Set file pointer
            fp->fptr = ofs;
            if (ofs > 0) {
                fp->clust = clmt_clust(fp, ofs - 1);
                dsc       = clst2sect(fs, fp->clust);
                if (dsc == 0) {
                    ABORT(fs, FR_INT_ERR);
                }
                dsc += (DWORD)((ofs - 1) / SS(fs)) & (fs->csize - 1);
                // Refill sector cache if needed
                if (fp->fptr % SS(fs) && dsc != fp->sect) {
#        if !FF_FS_TINY
#            if !FF_FS_READONLY
                    // Write-back dirty sector cache
                    if (fp->flag & FA_DIRTY) {
                        if (disk_write(fs->pdrv, fp->buf, fp->sect, 1)
                            != RES_OK) {
                            ABORT(fs, FR_DISK_ERR);
                        }
                        fp->flag &= (BYTE)~FA_DIRTY;
                    }
#            endif
                    // Load current sector
                    if (disk_read(fs->pdrv, fp->buf, dsc, 1) != RES_OK) {
                        ABORT(fs, FR_DISK_ERR);
                    }
#        endif
                    fp->sect = dsc;
                }
            }
        }
    }
    else
#    endif

    // Normal Seek

    {
#    if FF_FS_EXFAT
        // Clip at 4 GiB - 1 if at FATxx
        if (fs->fs_type != FS_EXFAT && ofs >= 0x100000000) {
            ofs = 0xFFFFFFFF;
        }
#    endif
        // In read-only mode, clip offset with the file size
        if (ofs > fp->obj.objsize
            && (FF_FS_READONLY || !(fp->flag & FA_WRITE))) {
            ofs = fp->obj.objsize;
        }
        ifptr    = fp->fptr;
        fp->fptr = nsect = 0;
        if (ofs > 0) {
            // Cluster size (byte)
            bcs = (DWORD)fs->csize * SS(fs);
            if (ifptr > 0 &&
                // When seek to same or following cluster,
                (ofs - 1) / bcs >= (ifptr - 1) / bcs) {
                // start from the current cluster
                fp->fptr  = (ifptr - 1) & ~(FSIZE_t)(bcs - 1);
                ofs      -= fp->fptr;
                clst      = fp->clust;
                // When seek to back cluster,
            }
            else {
                // start from the first cluster
                clst = fp->obj.sclust;
#    if !FF_FS_READONLY
                // If no cluster chain, create a new chain
                if (clst == 0) {
                    clst = create_chain(&fp->obj, 0);
                    if (clst == 1) {
                        ABORT(fs, FR_INT_ERR);
                    }
                    if (clst == 0xFFFFFFFF) {
                        ABORT(fs, FR_DISK_ERR);
                    }
                    fp->obj.sclust = clst;
                }
#    endif
                fp->clust = clst;
            }
            if (clst != 0) {
                // Cluster following loop
                while (ofs > bcs) {
                    ofs      -= bcs;
                    fp->fptr += bcs;
#    if !FF_FS_READONLY
                    // Check if in write mode or not
                    if (fp->flag & FA_WRITE) {
                        // No FAT chain object needs correct objsize to generate
                        // FAT value
                        if (FF_FS_EXFAT && fp->fptr > fp->obj.objsize) {
                            fp->obj.objsize  = fp->fptr;
                            fp->flag        |= FA_MODIFIED;
                        }
                        // Follow chain with forceed stretch
                        clst = create_chain(&fp->obj, clst);
                        // Clip file size in case of disk full
                        if (clst == 0) {
                            ofs = 0;
                            break;
                        }
                    }
                    else
#    endif
                    {
                        // Follow cluster chain if not in write mode
                        clst = get_fat(&fp->obj, clst);
                    }
                    if (clst == 0xFFFFFFFF) {
                        ABORT(fs, FR_DISK_ERR);
                    }
                    if (clst <= 1 || clst >= fs->n_fatent) {
                        ABORT(fs, FR_INT_ERR);
                    }
                    fp->clust = clst;
                }
                fp->fptr += ofs;
                if (ofs % SS(fs)) {
                    // Current sector
                    nsect = clst2sect(fs, clst);
                    if (nsect == 0) {
                        ABORT(fs, FR_INT_ERR);
                    }
                    nsect += (DWORD)(ofs / SS(fs));
                }
            }
        }
        // Set file change flag if the file size is extended
        if (!FF_FS_READONLY && fp->fptr > fp->obj.objsize) {
            fp->obj.objsize  = fp->fptr;
            fp->flag        |= FA_MODIFIED;
        }
        // Fill sector cache if needed
        if (fp->fptr % SS(fs) && nsect != fp->sect) {
#    if !FF_FS_TINY
#        if !FF_FS_READONLY
            // Write-back dirty sector cache
            if (fp->flag & FA_DIRTY) {
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#        endif
            // Fill sector cache
            if (disk_read(fs->pdrv, fp->buf, nsect, 1) != RES_OK) {
                ABORT(fs, FR_DISK_ERR);
            }
#    endif
            fp->sect = nsect;
        }
    }

    LEAVE_FF(fs, res);
}

#    if FF_FS_MINIMIZE <= 1
//-----------------------------------------------------------------------

// Create a Directory Object

//-----------------------------------------------------------------------

FRESULT f_opendir(DIR* dp, const TCHAR* path) {
    FRESULT res;
    FATFS*  fs;
    DEF_NAMBUF

    if (!dp) {
        return FR_INVALID_OBJECT;
    }

    // Get logical drive

    res = mount_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dp->obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the path to the directory
        res = follow_path(dp, path);
        // Follow completed
        if (res == FR_OK) {
            // It is not the origin directory itself
            if (!(dp->fn[NSFLAG] & NS_NONAME)) {
                // This object is a sub-directory
                if (dp->obj.attr & AM_DIR) {
#        if FF_FS_EXFAT
                    if (fs->fs_type == FS_EXFAT) {
                        // Get containing directory inforamation
                        dp->obj.c_scl  = dp->obj.sclust;
                        dp->obj.c_size = ((DWORD)dp->obj.objsize & 0xFFFFFF00)
                                       | dp->obj.stat;
                        dp->obj.c_ofs = dp->blk_ofs;
                        // Get object allocation info
                        init_alloc_info(fs, &dp->obj);
                    }
                    else
#        endif
                    {
                        // Get object allocation info
                        dp->obj.sclust = ld_clust(fs, dp->dir);
                    }
                    // This object is a file
                }
                else {
                    res = FR_NO_PATH;
                }
            }
            if (res == FR_OK) {
                dp->obj.id = fs->id;
                // Rewind directory
                res        = dir_sdi(dp, 0);
#        if FF_FS_LOCK
                if (res == FR_OK) {
                    if (dp->obj.sclust != 0) {
                        // Lock the sub directory
                        dp->obj.lockid = inc_share(dp, 0);
                        if (!dp->obj.lockid) {
                            res = FR_TOO_MANY_OPEN_FILES;
                        }
                    }
                    else {
                        // Root directory need not to be locked
                        dp->obj.lockid = 0;
                    }
                }
#        endif
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE) {
            res = FR_NO_PATH;
        }
    }
    // Invalidate the directory object if function failed
    if (res != FR_OK) {
        dp->obj.fs = 0;
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Close Directory

//-----------------------------------------------------------------------

FRESULT f_closedir(DIR* dp) {
    FRESULT res;
    FATFS*  fs;

    // Check validity of the file object
    res = validate(&dp->obj, &fs);
    if (res == FR_OK) {
#        if FF_FS_LOCK
        // Decrement sub-directory open counter
        if (dp->obj.lockid) {
            res = dec_share(dp->obj.lockid);
        }
        // Invalidate directory object
        if (res == FR_OK) {
            dp->obj.fs = 0;
        }
#        else
        // Invalidate directory object
        dp->obj.fs = 0;
#        endif
#        if FF_FS_REENTRANT
        // Unlock volume
        unlock_volume(fs, FR_OK);
#        endif
    }
    return res;
}

//-----------------------------------------------------------------------

// Read Directory Entries in Sequence

//-----------------------------------------------------------------------

FRESULT f_readdir(DIR* dp, FILINFO* fno) {
    FRESULT res;
    FATFS*  fs;
    DEF_NAMBUF

    // Check validity of the directory object
    res = validate(&dp->obj, &fs);
    if (res == FR_OK) {
        if (!fno) {
            // Rewind the directory object
            res = dir_sdi(dp, 0);
        }
        else {
            INIT_NAMBUF(fs);
            // Read an item
            res = DIR_READ_FILE(dp);
            // Ignore end of directory
            if (res == FR_NO_FILE) {
                res = FR_OK;
            }
            // A valid entry is found
            if (res == FR_OK) {
                // Get the object information
                get_fileinfo(dp, fno);
                // Increment index for next
                res = dir_next(dp, 0);
                // Ignore end of directory now
                if (res == FR_NO_FILE) {
                    res = FR_OK;
                }
            }
            FREE_NAMBUF();
        }
    }
    LEAVE_FF(fs, res);
}

#        if FF_USE_FIND
//-----------------------------------------------------------------------

// Find Next File

//-----------------------------------------------------------------------

FRESULT f_findnext(DIR* dp, FILINFO* fno) {
    FRESULT res;

    for (;;) {
        // Get a directory item
        res = f_readdir(dp, fno);
        // Terminate if any error or end of directory
        if (res != FR_OK || !fno || !fno->fname[0]) {
            break;
        }
        // Test for the file name
        if (pattern_match(dp->pat, fno->fname, 0, FIND_RECURS)) {
            break;
        }
#            if FF_USE_LFN && FF_USE_FIND == 2
        // Test for alternative name if exist
        if (pattern_match(dp->pat, fno->altname, 0, FIND_RECURS)) {
            break;
        }
#            endif
    }
    return res;
}

//-----------------------------------------------------------------------

// Find First File

//-----------------------------------------------------------------------

FRESULT
f_findfirst(DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern) {
    FRESULT res;

    // Save pointer to pattern string
    dp->pat = pattern;
    // Open the target directory
    res     = f_opendir(dp, path);
    if (res == FR_OK) {
        // Find the first item
        res = f_findnext(dp, fno);
    }
    return res;
}

// FF_USE_FIND
#        endif

#        if FF_FS_MINIMIZE == 0
//-----------------------------------------------------------------------

// Get File Status

//-----------------------------------------------------------------------

FRESULT f_stat(const TCHAR* path, FILINFO* fno) {
    FRESULT res;
    DIR     dj;
    DEF_NAMBUF

    // Get logical drive

    res = mount_volume(&path, &dj.obj.fs, 0);
    if (res == FR_OK) {
        INIT_NAMBUF(dj.obj.fs);
        // Follow the file path
        res = follow_path(&dj, path);
        // Follow completed
        if (res == FR_OK) {
            // It is origin directory
            if (dj.fn[NSFLAG] & NS_NONAME) {
                res = FR_INVALID_NAME;
                // Found an object
            }
            else {
                if (fno) {
                    get_fileinfo(&dj, fno);
                }
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(dj.obj.fs, res);
}

#            if !FF_FS_READONLY
//-----------------------------------------------------------------------

// Get Number of Free Clusters

//-----------------------------------------------------------------------

FRESULT f_getfree(const TCHAR* path, DWORD* nclst, FATFS** fatfs) {
    FRESULT res;
    FATFS*  fs;
    DWORD   nfree, clst, stat;
    LBA_t   sect;
    UINT    i;
    FFOBJID obj;

    // Get logical drive

    res = mount_volume(&path, &fs, 0);
    if (res == FR_OK) {
        // Return ptr to the fs object
        *fatfs = fs;
        // If free_clst is valid, return it without full FAT scan

        if (fs->free_clst <= fs->n_fatent - 2) {
            *nclst = fs->free_clst;
        }
        else {
            // Scan FAT to obtain number of free clusters

            nfree = 0;
            // FAT12: Scan bit field FAT entries
            if (fs->fs_type == FS_FAT12) {
                clst   = 2;
                obj.fs = fs;
                do {
                    stat = get_fat(&obj, clst);
                    if (stat == 0xFFFFFFFF) {
                        res = FR_DISK_ERR;
                        break;
                    }
                    if (stat == 1) {
                        res = FR_INT_ERR;
                        break;
                    }
                    if (stat == 0) {
                        nfree++;
                    }
                } while (++clst < fs->n_fatent);
            }
            else {
#                if FF_FS_EXFAT
                // exFAT: Scan allocation bitmap
                if (fs->fs_type == FS_EXFAT) {
                    BYTE bm;
                    UINT b;

                    // Number of clusters
                    clst = fs->n_fatent - 2;
                    // Bitmap sector
                    sect = fs->bitbase;
                    // Offset in the sector
                    i    = 0;
                    // Counts numbuer of bits with zero in the bitmap
                    do {
                        // New sector?
                        if (i == 0) {
                            res = move_window(fs, sect++);
                            if (res != FR_OK) {
                                break;
                            }
                        }
                        for (b = 8, bm = ~fs->win[i]; b && clst; b--, clst--) {
                            nfree  += bm & 1;
                            bm    >>= 1;
                        }
                        i = (i + 1) % SS(fs);
                    } while (clst);
                }
                else
#                endif
                // FAT16/32: Scan WORD/DWORD FAT entries
                {
                    // Number of entries
                    clst = fs->n_fatent;
                    // Top of the FAT
                    sect = fs->fatbase;
                    // Offset in the sector
                    i    = 0;
                    // Counts numbuer of entries with zero in the FAT
                    do {
                        // New sector?
                        if (i == 0) {
                            res = move_window(fs, sect++);
                            if (res != FR_OK) {
                                break;
                            }
                        }
                        if (fs->fs_type == FS_FAT16) {
                            if (ld_word(fs->win + i) == 0) {
                                nfree++;
                            }
                            i += 2;
                        }
                        else {
                            if ((ld_dword(fs->win + i) & 0x0FFFFFFF) == 0) {
                                nfree++;
                            }
                            i += 4;
                        }
                        i %= SS(fs);
                    } while (--clst);
                }
            }
            // Update parameters if succeeded
            if (res == FR_OK) {
                // Return the free clusters
                *nclst         = nfree;
                // Now free_clst is valid
                fs->free_clst  = nfree;
                // FAT32: FSInfo is to be updated
                fs->fsi_flag  |= 1;
            }
        }
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Truncate File

//-----------------------------------------------------------------------

FRESULT f_truncate(FIL* fp) {
    FRESULT res;
    FATFS*  fs;
    DWORD   ncl;

    // Check validity of the file object
    res = validate(&fp->obj, &fs);
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) {
        LEAVE_FF(fs, res);
    }
    // Check access mode
    if (!(fp->flag & FA_WRITE)) {
        LEAVE_FF(fs, FR_DENIED);
    }

    // Process when fptr is not on the eof
    if (fp->fptr < fp->obj.objsize) {
        // When set file size to zero, remove entire cluster chain
        if (fp->fptr == 0) {
            res            = remove_chain(&fp->obj, fp->obj.sclust, 0);
            fp->obj.sclust = 0;
            // When truncate a part of the file, remove remaining clusters
        }
        else {
            ncl = get_fat(&fp->obj, fp->clust);
            res = FR_OK;
            if (ncl == 0xFFFFFFFF) {
                res = FR_DISK_ERR;
            }
            if (ncl == 1) {
                res = FR_INT_ERR;
            }
            if (res == FR_OK && ncl < fs->n_fatent) {
                res = remove_chain(&fp->obj, ncl, fp->clust);
            }
        }
        // Set file size to current read/write point
        fp->obj.objsize  = fp->fptr;
        fp->flag        |= FA_MODIFIED;
#                if !FF_FS_TINY
        if (res == FR_OK && (fp->flag & FA_DIRTY)) {
            if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                res = FR_DISK_ERR;
            }
            else {
                fp->flag &= (BYTE)~FA_DIRTY;
            }
        }
#                endif
        if (res != FR_OK) {
            ABORT(fs, res);
        }
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Delete a File/Directory

//-----------------------------------------------------------------------

FRESULT f_unlink(const TCHAR* path) {
    FRESULT res;
    FATFS*  fs;
    DIR     dj, sdj;
    DWORD   dclst = 0;
#                if FF_FS_EXFAT
    FFOBJID obj;
#                endif
    DEF_NAMBUF

    // Get logical drive

    res = mount_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the file path
        res = follow_path(&dj, path);
        if (FF_FS_RPATH && res == FR_OK && (dj.fn[NSFLAG] & NS_DOT)) {
            // Cannot remove dot entry
            res = FR_INVALID_NAME;
        }
#                if FF_FS_LOCK
        // Check if it is an open object
        if (res == FR_OK) {
            res = chk_share(&dj, 2);
        }
#                endif
        // The object is accessible
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) {
                // Cannot remove the origin directory
                res = FR_INVALID_NAME;
            }
            else {
                if (dj.obj.attr & AM_RDO) {
                    // Cannot remove R/O object
                    res = FR_DENIED;
                }
            }
            if (res == FR_OK) {
#                if FF_FS_EXFAT
                obj.fs = fs;
                if (fs->fs_type == FS_EXFAT) {
                    init_alloc_info(fs, &obj);
                    dclst = obj.sclust;
                }
                else
#                endif
                {
                    dclst = ld_clust(fs, dj.dir);
                }
                // Is it a sub-directory?
                if (dj.obj.attr & AM_DIR) {
#                if FF_FS_RPATH != 0
                    // Is it the current directory?
                    if (dclst == fs->cdir) {
                        res = FR_DENIED;
                    }
                    else
#                endif
                    {
                        // Open the sub-directory
                        sdj.obj.fs     = fs;
                        sdj.obj.sclust = dclst;
#                if FF_FS_EXFAT
                        if (fs->fs_type == FS_EXFAT) {
                            sdj.obj.objsize = obj.objsize;
                            sdj.obj.stat    = obj.stat;
                        }
#                endif
                        res = dir_sdi(&sdj, 0);
                        if (res == FR_OK) {
                            // Test if the directory is empty
                            res = DIR_READ_FILE(&sdj);
                            // Not empty?
                            if (res == FR_OK) {
                                res = FR_DENIED;
                            }
                            // Empty?
                            if (res == FR_NO_FILE) {
                                res = FR_OK;
                            }
                        }
                    }
                }
            }
            if (res == FR_OK) {
                // Remove the directory entry
                res = dir_remove(&dj);
                // Remove the cluster chain if exist
                if (res == FR_OK && dclst != 0) {
#                if FF_FS_EXFAT
                    res = remove_chain(&obj, dclst, 0);
#                else
                    res = remove_chain(&dj.obj, dclst, 0);
#                endif
                }
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Create a Directory

//-----------------------------------------------------------------------

FRESULT f_mkdir(const TCHAR* path) {
    FRESULT res;
    FATFS*  fs;
    DIR     dj;
    FFOBJID sobj;
    DWORD   dcl, pcl, tm;
    DEF_NAMBUF

    // Get logical drive
    res = mount_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the file path
        res = follow_path(&dj, path);
        // Name collision?
        if (res == FR_OK) {
            res = FR_EXIST;
        }
        // Invalid name?
        if (FF_FS_RPATH && res == FR_NO_FILE && (dj.fn[NSFLAG] & NS_DOT)) {
            res = FR_INVALID_NAME;
        }
        // It is clear to create a new directory
        if (res == FR_NO_FILE) {
            // New object id to create a new chain
            sobj.fs = fs;
            // Allocate a cluster for the new directory
            dcl     = create_chain(&sobj, 0);
            res     = FR_OK;
            // No space to allocate a new cluster?
            if (dcl == 0) {
                res = FR_DENIED;
            }
            // Any insanity?
            if (dcl == 1) {
                res = FR_INT_ERR;
            }
            // Disk error?
            if (dcl == 0xFFFFFFFF) {
                res = FR_DISK_ERR;
            }
            tm = GET_FATTIME();
            if (res == FR_OK) {
                // Clean up the new table
                res = dir_clear(fs, dcl);
                if (res == FR_OK) {
                    // Create dot entries (FAT only)
                    if (!FF_FS_EXFAT || fs->fs_type != FS_EXFAT) {
                        // Create "." entry
                        memset(fs->win + DIR_Name, ' ', 11);
                        fs->win[DIR_Name] = '.';
                        fs->win[DIR_Attr] = AM_DIR;
                        st_dword(fs->win + DIR_ModTime, tm);
                        st_clust(fs, fs->win, dcl);
                        // Create ".." entry
                        memcpy(fs->win + SZDIRE, fs->win, SZDIRE);
                        fs->win[SZDIRE + 1] = '.';
                        pcl                 = dj.obj.sclust;
                        st_clust(fs, fs->win + SZDIRE, pcl);
                        fs->wflag = 1;
                    }
                    // Register the object to the parent directoy
                    res = dir_register(&dj);
                }
            }
            if (res == FR_OK) {
#                if FF_FS_EXFAT
                // Initialize directory entry block
                if (fs->fs_type == FS_EXFAT) {
                    // Created time
                    st_dword(fs->dirbuf + XDIR_ModTime, tm);
                    // Table start cluster
                    st_dword(fs->dirbuf + XDIR_FstClus, dcl);
                    // Directory size needs to be valid
                    st_dword(fs->dirbuf + XDIR_FileSize,
                             (DWORD)fs->csize * SS(fs));
                    st_dword(fs->dirbuf + XDIR_ValidFileSize,
                             (DWORD)fs->csize * SS(fs));
                    // Initialize the object flag
                    fs->dirbuf[XDIR_GenFlags] = 3;
                    // Attribute
                    fs->dirbuf[XDIR_Attr]     = AM_DIR;
                    res                       = store_xdir(&dj);
                }
                else
#                endif
                {
                    // Created time
                    st_dword(dj.dir + DIR_ModTime, tm);
                    // Table start cluster
                    st_clust(fs, dj.dir, dcl);
                    // Attribute
                    dj.dir[DIR_Attr] = AM_DIR;
                    fs->wflag        = 1;
                }
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            }
            else {
                // Could not register, remove the allocated cluster
                remove_chain(&sobj, dcl, 0);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Rename a File/Directory

//-----------------------------------------------------------------------

FRESULT f_rename(const TCHAR* path_old, const TCHAR* path_new) {
    FRESULT res;
    FATFS*  fs;
    DIR     djo, djn;
    BYTE    buf[FF_FS_EXFAT ? SZDIRE * 2 : SZDIRE], *dir;
    LBA_t   sect;
    DEF_NAMBUF

    // Snip the drive number of new name off
    get_ldnumber(&path_new);
    // Get logical drive of the old object
    res = mount_volume(&path_old, &fs, FA_WRITE);
    if (res == FR_OK) {
        djo.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Check old object
        res = follow_path(&djo, path_old);
        // Check validity of name
        if (res == FR_OK && (djo.fn[NSFLAG] & (NS_DOT | NS_NONAME))) {
            res = FR_INVALID_NAME;
        }
#                if FF_FS_LOCK
        if (res == FR_OK) {
            res = chk_share(&djo, 2);
        }
#                endif
        // Object to be renamed is found
        if (res == FR_OK) {
#                if FF_FS_EXFAT
            // At exFAT volume
            if (fs->fs_type == FS_EXFAT) {
                BYTE nf, nn;
                WORD nh;

                // Save 85+C0 entry of old object
                memcpy(buf, fs->dirbuf, SZDIRE * 2);
                memcpy(&djn, &djo, sizeof djo);
                // Make sure if new object name is not in use
                res = follow_path(&djn, path_new);
                // Is new name already in use by any other object?
                if (res == FR_OK) {
                    res = (djn.obj.sclust == djo.obj.sclust
                           && djn.dptr == djo.dptr)
                          ? FR_NO_FILE
                          : FR_EXIST;
                }
                // It is a valid path and no name collision
                if (res == FR_NO_FILE) {
                    // Register the new entry
                    res = dir_register(&djn);
                    if (res == FR_OK) {
                        nf = fs->dirbuf[XDIR_NumSec];
                        nn = fs->dirbuf[XDIR_NumName];
                        nh = ld_word(fs->dirbuf + XDIR_NameHash);
                        // Restore 85+C0 entry
                        memcpy(fs->dirbuf, buf, SZDIRE * 2);
                        fs->dirbuf[XDIR_NumSec]  = nf;
                        fs->dirbuf[XDIR_NumName] = nn;
                        st_word(fs->dirbuf + XDIR_NameHash, nh);
                        // Set archive attribute if it is a file
                        if (!(fs->dirbuf[XDIR_Attr] & AM_DIR)) {
                            fs->dirbuf[XDIR_Attr] |= AM_ARC;
                        }
                        // Start of critical section where an interruption can
                        // cause a cross-link

                        res = store_xdir(&djn);
                    }
                }
            }
            else
#                endif
            // At FAT/FAT32 volume
            {
                // Save directory entry of the object
                memcpy(buf, djo.dir, SZDIRE);
                // Duplicate the directory object
                memcpy(&djn, &djo, sizeof(DIR));
                // Make sure if new object name is not in use
                res = follow_path(&djn, path_new);
                // Is new name already in use by any other object?
                if (res == FR_OK) {
                    res = (djn.obj.sclust == djo.obj.sclust
                           && djn.dptr == djo.dptr)
                          ? FR_NO_FILE
                          : FR_EXIST;
                }
                // It is a valid path and no name collision
                if (res == FR_NO_FILE) {
                    // Register the new entry
                    res = dir_register(&djn);
                    if (res == FR_OK) {
                        // Copy directory entry of the object except name
                        dir = djn.dir;
                        memcpy(dir + 13, buf + 13, SZDIRE - 13);
                        dir[DIR_Attr] = buf[DIR_Attr];
                        // Set archive attribute if it is a file
                        if (!(dir[DIR_Attr] & AM_DIR)) {
                            dir[DIR_Attr] |= AM_ARC;
                        }
                        fs->wflag = 1;
                        // Update .. entry in the sub-directory if needed
                        if ((dir[DIR_Attr] & AM_DIR)
                            && djo.obj.sclust != djn.obj.sclust) {
                            sect = clst2sect(fs, ld_clust(fs, dir));
                            if (sect == 0) {
                                res = FR_INT_ERR;
                            }
                            else {
                                // Start of critical section where an
                                // interruption can cause a cross-link

                                res = move_window(fs, sect);
                                // Ptr to .. entry
                                dir = fs->win + SZDIRE * 1;
                                if (res == FR_OK && dir[1] == '.') {
                                    st_clust(fs, dir, djn.obj.sclust);
                                    fs->wflag = 1;
                                }
                            }
                        }
                    }
                }
            }
            if (res == FR_OK) {
                // Remove old entry
                res = dir_remove(&djo);
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            }
            // End of the critical section
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

// !FF_FS_READONLY
#            endif
// FF_FS_MINIMIZE == 0
#        endif
// FF_FS_MINIMIZE <= 1
#    endif
// FF_FS_MINIMIZE <= 2
#endif

#if FF_USE_CHMOD && !FF_FS_READONLY
//-----------------------------------------------------------------------

// Change Attribute

//-----------------------------------------------------------------------

FRESULT f_chmod(const TCHAR* path, BYTE attr, BYTE mask) {
    FRESULT res;
    FATFS*  fs;
    DIR     dj;
    DEF_NAMBUF

    // Get logical drive
    res = mount_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the file path
        res = follow_path(&dj, path);
        // Check object validity
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) {
            res = FR_INVALID_NAME;
        }
        if (res == FR_OK) {
            // Valid attribute mask
            mask &= AM_RDO | AM_HID | AM_SYS | AM_ARC;
#    if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                // Apply attribute change
                fs->dirbuf[XDIR_Attr]
                  = (attr & mask) | (fs->dirbuf[XDIR_Attr] & (BYTE)~mask);
                res = store_xdir(&dj);
            }
            else
#    endif
            {
                // Apply attribute change
                dj.dir[DIR_Attr]
                  = (attr & mask) | (dj.dir[DIR_Attr] & (BYTE)~mask);
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

//-----------------------------------------------------------------------

// Change Timestamp

//-----------------------------------------------------------------------

FRESULT f_utime(const TCHAR* path, const FILINFO* fno) {
    FRESULT res;
    FATFS*  fs;
    DIR     dj;
    DEF_NAMBUF

    // Get logical drive
    res = mount_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        // Follow the file path
        res = follow_path(&dj, path);
        // Check object validity
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME))) {
            res = FR_INVALID_NAME;
        }
        if (res == FR_OK) {
#    if FF_FS_EXFAT
            if (fs->fs_type == FS_EXFAT) {
                st_dword(fs->dirbuf + XDIR_ModTime,
                         (DWORD)fno->fdate << 16 | fno->ftime);
                res = store_xdir(&dj);
            }
            else
#    endif
            {
                st_dword(dj.dir + DIR_ModTime,
                         (DWORD)fno->fdate << 16 | fno->ftime);
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

// FF_USE_CHMOD && !FF_FS_READONLY
#endif

#if FF_USE_LABEL
//-----------------------------------------------------------------------

// Get Volume Label

//-----------------------------------------------------------------------

FRESULT f_getlabel(const TCHAR* path, TCHAR* label, DWORD* vsn) {
    FRESULT res;
    FATFS*  fs;
    DIR     dj;
    UINT    si, di;
    WCHAR   wc;

    // Get logical drive

    res = mount_volume(&path, &fs, 0);

    // Get volume label

    if (res == FR_OK && label) {
        // Open root directory
        dj.obj.fs     = fs;
        dj.obj.sclust = 0;
        res           = dir_sdi(&dj, 0);
        if (res == FR_OK) {
            // Find a volume label entry
            res = DIR_READ_LABEL(&dj);
            if (res == FR_OK) {
#    if FF_FS_EXFAT
                if (fs->fs_type == FS_EXFAT) {
                    WCHAR hs;
                    UINT  nw;

                    // Extract volume label from 83 entry
                    for (si = di = hs = 0; si < dj.dir[XDIR_NumLabel]; si++) {
                        wc = ld_word(dj.dir + XDIR_Label + si * 2);
                        // Is the code a surrogate?
                        if (hs == 0 && IsSurrogate(wc)) {
                            hs = wc;
                            continue;
                        }
                        // Store it in API encoding
                        nw = put_utf((DWORD)hs << 16 | wc, &label[di], 4);
                        // Encode error?
                        if (nw == 0) {
                            di = 0;
                            break;
                        }
                        di += nw;
                        hs  = 0;
                    }
                    // Broken surrogate pair?
                    if (hs != 0) {
                        di = 0;
                    }
                    label[di] = 0;
                }
                else
#    endif
                {
                    // Extract volume label from AM_VOL entry
                    si = di = 0;
                    while (si < 11) {
                        wc = dj.dir[si++];
                        // Unicode output
#    if FF_USE_LFN && FF_LFN_UNICODE >= 1
                        // Is it a DBC?
                        if (dbc_1st((BYTE)wc) && si < 11) {
                            wc = wc << 8 | dj.dir[si++];
                        }
                        // Convert it into Unicode
                        wc = ff_oem2uni(wc, CODEPAGE);
                        // Invalid char in current code page?
                        if (wc == 0) {
                            di = 0;
                            break;
                        }
                        // Store it in Unicode
                        di += put_utf(wc, &label[di], 4);
                        // ANSI/OEM output
#    else
                        label[di++] = (TCHAR)wc;
#    endif
                    }
                    // Truncate trailing spaces
                    do {
                        label[di] = 0;
                        if (di == 0) {
                            break;
                        }
                    } while (label[--di] == ' ');
                }
            }
        }
        // No label entry and return nul string
        if (res == FR_NO_FILE) {
            label[0] = 0;
            res      = FR_OK;
        }
    }

    // Get volume serial number

    if (res == FR_OK && vsn) {
        res = move_window(fs, fs->volbase);
        if (res == FR_OK) {
            switch (fs->fs_type) {
                case FS_EXFAT:
                    di = BPB_VolIDEx;
                    break;

                case FS_FAT32:
                    di = BS_VolID32;
                    break;

                default:
                    di = BS_VolID;
            }
            *vsn = ld_dword(fs->win + di);
        }
    }

    LEAVE_FF(fs, res);
}

#    if !FF_FS_READONLY
//-----------------------------------------------------------------------

// Set Volume Label

//-----------------------------------------------------------------------

FRESULT f_setlabel(const TCHAR* label) {
    FRESULT           res;
    FATFS*            fs;
    DIR               dj;
    BYTE              dirvn[22];
    UINT              di;
    WCHAR             wc;
    // [0..16] for FAT, [7..16] for exFAT
    static const char badchr[18] = "+.,;=[]"
                                   "/*:<>|\\\"\?\x7F";

#        if FF_USE_LFN
    DWORD dc;
#        endif

    // Get logical drive

    res = mount_volume(&label, &fs, FA_WRITE);
    if (res != FR_OK) {
        LEAVE_FF(fs, res);
    }

#        if FF_FS_EXFAT
    // On the exFAT volume
    if (fs->fs_type == FS_EXFAT) {
        memset(dirvn, 0, 22);
        di = 0;
        // Create volume label
        while ((UINT)*label >= ' ') {
            // Get a Unicode character
            dc = tchar2uni(&label);
            if (dc >= 0x10000) {
                // Wrong surrogate or buffer overflow
                if (dc == 0xFFFFFFFF || di >= 10) {
                    dc = 0;
                }
                else {
                    st_word(dirvn + di * 2, (WCHAR)(dc >> 16));
                    di++;
                }
            }
            // Check validity of the volume label
            if (dc == 0 || strchr(&badchr[7], (int)dc) || di >= 11) {
                LEAVE_FF(fs, FR_INVALID_NAME);
            }
            st_word(dirvn + di * 2, (WCHAR)dc);
            di++;
        }
    }
    else
#        endif
    // On the FAT/FAT32 volume
    {
        memset(dirvn, ' ', 11);
        di = 0;
        // Create volume label
        while ((UINT)*label >= ' ') {
#        if FF_USE_LFN
            dc = tchar2uni(&label);
            wc = (dc < 0x10000) ? ff_uni2oem(ff_wtoupper(dc), CODEPAGE) : 0;
            // ANSI/OEM input
#        else
            wc = (BYTE)*label++;
            if (dbc_1st((BYTE)wc)) {
                wc = dbc_2nd((BYTE)*label) ? wc << 8 | (BYTE)*label++ : 0;
            }
            // To upper ASCII characters
            if (IsLower(wc)) {
                wc -= 0x20;
            }
#            if FF_CODE_PAGE == 0
            // To upper extended characters (SBCS cfg)
            if (ExCvt && wc >= 0x80) {
                wc = ExCvt[wc - 0x80];
            }
#            elif FF_CODE_PAGE < 900
            // To upper extended characters (SBCS cfg)
            if (wc >= 0x80) {
                wc = ExCvt[wc - 0x80];
            }
#            endif
#        endif
            // Reject invalid characters for volume label
            if (wc == 0 || strchr(&badchr[0], (int)wc)
                || di >= (UINT)((wc >= 0x100) ? 10 : 11)) {
                LEAVE_FF(fs, FR_INVALID_NAME);
            }
            if (wc >= 0x100) {
                dirvn[di++] = (BYTE)(wc >> 8);
            }
            dirvn[di++] = (BYTE)wc;
        }
        // Reject illegal name (heading DDEM)
        if (dirvn[0] == DDEM) {
            LEAVE_FF(fs, FR_INVALID_NAME);
        }
        // Snip trailing spaces
        while (di && dirvn[di - 1] == ' ') {
            di--;
        }
    }

    // Set volume label

    // Open root directory
    dj.obj.fs     = fs;
    dj.obj.sclust = 0;
    res           = dir_sdi(&dj, 0);
    if (res == FR_OK) {
        // Get volume label entry
        res = DIR_READ_LABEL(&dj);
        if (res == FR_OK) {
            if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
                // Change the volume label
                dj.dir[XDIR_NumLabel] = (BYTE)di;
                memcpy(dj.dir + XDIR_Label, dirvn, 22);
            }
            else {
                if (di != 0) {
                    // Change the volume label
                    memcpy(dj.dir, dirvn, 11);
                }
                else {
                    // Remove the volume label
                    dj.dir[DIR_Name] = DDEM;
                }
            }
            fs->wflag = 1;
            res       = sync_fs(fs);
            // No volume label entry or an error
        }
        else {
            if (res == FR_NO_FILE) {
                res = FR_OK;
                // Create a volume label entry
                if (di != 0) {
                    // Allocate an entry
                    res = dir_alloc(&dj, 1);
                    if (res == FR_OK) {
                        // Clean the entry
                        memset(dj.dir, 0, SZDIRE);
                        if (FF_FS_EXFAT && fs->fs_type == FS_EXFAT) {
                            // Create volume label entry
                            dj.dir[XDIR_Type]     = ET_VLABEL;
                            dj.dir[XDIR_NumLabel] = (BYTE)di;
                            memcpy(dj.dir + XDIR_Label, dirvn, 22);
                        }
                        else {
                            // Create volume label entry
                            dj.dir[DIR_Attr] = AM_VOL;
                            memcpy(dj.dir, dirvn, 11);
                        }
                        fs->wflag = 1;
                        res       = sync_fs(fs);
                    }
                }
            }
        }
    }

    LEAVE_FF(fs, res);
}

// !FF_FS_READONLY
#    endif
// FF_USE_LABEL
#endif

#if FF_USE_EXPAND && !FF_FS_READONLY
//-----------------------------------------------------------------------

// Allocate a Contiguous Blocks to the File

//-----------------------------------------------------------------------

FRESULT f_expand(FIL* fp, FSIZE_t fsz, BYTE opt) {
    FRESULT res;
    FATFS*  fs;
    DWORD   n, clst, stcl, scl, ncl, tcl, lclst;

    // Check validity of the file object
    res = validate(&fp->obj, &fs);
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) {
        LEAVE_FF(fs, res);
    }
    if (fsz == 0 || fp->obj.objsize != 0 || !(fp->flag & FA_WRITE)) {
        LEAVE_FF(fs, FR_DENIED);
    }
#    if FF_FS_EXFAT
    // Check if in size limit
    if (fs->fs_type != FS_EXFAT && fsz >= 0x100000000) {
        LEAVE_FF(fs, FR_DENIED);
    }
#    endif
    // Cluster size
    n     = (DWORD)fs->csize * SS(fs);
    // Number of clusters required
    tcl   = (DWORD)(fsz / n) + ((fsz & (n - 1)) ? 1 : 0);
    stcl  = fs->last_clst;
    lclst = 0;
    if (stcl < 2 || stcl >= fs->n_fatent) {
        stcl = 2;
    }

#    if FF_FS_EXFAT
    if (fs->fs_type == FS_EXFAT) {
        // Find a contiguous cluster block
        scl = find_bitmap(fs, stcl, tcl);
        // No contiguous cluster block was found
        if (scl == 0) {
            res = FR_DENIED;
        }
        if (scl == 0xFFFFFFFF) {
            res = FR_DISK_ERR;
        }
        // A contiguous free area is found
        if (res == FR_OK) {
            // Allocate it now
            if (opt) {
                // Mark the cluster block 'in use'
                res   = change_bitmap(fs, scl, tcl, 1);
                lclst = scl + tcl - 1;
                // Set it as suggested point for next allocation
            }
            else {
                lclst = scl - 1;
            }
        }
    }
    else
#    endif
    {
        scl = clst = stcl;
        ncl        = 0;
        // Find a contiguous cluster block
        for (;;) {
            n = get_fat(&fp->obj, clst);
            if (++clst >= fs->n_fatent) {
                clst = 2;
            }
            if (n == 1) {
                res = FR_INT_ERR;
                break;
            }
            if (n == 0xFFFFFFFF) {
                res = FR_DISK_ERR;
                break;
            }
            // Is it a free cluster?
            if (n == 0) {
                // Break if a contiguous cluster block is found
                if (++ncl == tcl) {
                    break;
                }
            }
            else {
                // Not a free cluster
                scl = clst;
                ncl = 0;
            }
            // No contiguous cluster?
            if (clst == stcl) {
                res = FR_DENIED;
                break;
            }
        }
        // A contiguous free area is found
        if (res == FR_OK) {
            // Allocate it now
            if (opt) {
                // Create a cluster chain on the FAT
                for (clst = scl, n = tcl; n; clst++, n--) {
                    res = put_fat(fs, clst, (n == 1) ? 0xFFFFFFFF : clst + 1);
                    if (res != FR_OK) {
                        break;
                    }
                    lclst = clst;
                }
                // Set it as suggested point for next allocation
            }
            else {
                lclst = scl - 1;
            }
        }
    }

    if (res == FR_OK) {
        // Set suggested start cluster to start next
        fs->last_clst = lclst;
        // Is it allocated now?
        if (opt) {
            // Update object allocation information
            fp->obj.sclust  = scl;
            fp->obj.objsize = fsz;
            // Set status 'contiguous chain'
            if (FF_FS_EXFAT) {
                fp->obj.stat = 2;
            }
            fp->flag |= FA_MODIFIED;
            // Update FSINFO
            if (fs->free_clst <= fs->n_fatent - 2) {
                fs->free_clst -= tcl;
                fs->fsi_flag  |= 1;
            }
        }
    }

    LEAVE_FF(fs, res);
}

// FF_USE_EXPAND && !FF_FS_READONLY
#endif

#if FF_USE_FORWARD
//-----------------------------------------------------------------------

// Forward Data to the Stream Directly

//-----------------------------------------------------------------------

FRESULT
f_forward(FIL* fp, UINT (*func)(const BYTE*, UINT), UINT btf, UINT* bf) {
    FRESULT res;
    FATFS*  fs;
    DWORD   clst;
    LBA_t   sect;
    FSIZE_t remain;
    UINT    rcnt, csect;
    BYTE*   dbuf;

    // Clear transfer byte counter
    *bf = 0;
    // Check validity of the file object
    res = validate(&fp->obj, &fs);
    if (res != FR_OK || (res = (FRESULT)fp->err) != FR_OK) {
        LEAVE_FF(fs, res);
    }
    // Check access mode
    if (!(fp->flag & FA_READ)) {
        LEAVE_FF(fs, FR_DENIED);
    }

    remain = fp->obj.objsize - fp->fptr;
    // Truncate btf by remaining bytes
    if (btf > remain) {
        btf = (UINT)remain;
    }

    // Repeat until all data transferred or stream goes busy
    for (; btf > 0 && (*func)(0, 0);
         fp->fptr += rcnt, *bf += rcnt, btf -= rcnt) {
        // Sector offset in the cluster
        csect = (UINT)(fp->fptr / SS(fs) & (fs->csize - 1));
        // On the sector boundary?
        if (fp->fptr % SS(fs) == 0) {
            // On the cluster boundary?
            if (csect == 0) {
                // On the top of the file?
                clst = (fp->fptr == 0) ? fp->obj.sclust
                                       : get_fat(&fp->obj, fp->clust);
                if (clst <= 1) {
                    ABORT(fs, FR_INT_ERR);
                }
                if (clst == 0xFFFFFFFF) {
                    ABORT(fs, FR_DISK_ERR);
                }
                // Update current cluster
                fp->clust = clst;
            }
        }
        // Get current data sector
        sect = clst2sect(fs, fp->clust);
        if (sect == 0) {
            ABORT(fs, FR_INT_ERR);
        }
        sect += csect;
#    if FF_FS_TINY
        // Move sector window to the file data
        if (move_window(fs, sect) != FR_OK) {
            ABORT(fs, FR_DISK_ERR);
        }
        dbuf = fs->win;
#    else
        // Fill sector cache with file data
        if (fp->sect != sect) {
#        if !FF_FS_READONLY
            // Write-back dirty sector cache
            if (fp->flag & FA_DIRTY) {
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != RES_OK) {
                    ABORT(fs, FR_DISK_ERR);
                }
                fp->flag &= (BYTE)~FA_DIRTY;
            }
#        endif
            if (disk_read(fs->pdrv, fp->buf, sect, 1) != RES_OK) {
                ABORT(fs, FR_DISK_ERR);
            }
        }
        dbuf = fp->buf;
#    endif
        fp->sect = sect;
        // Number of bytes remains in the sector
        rcnt     = SS(fs) - (UINT)fp->fptr % SS(fs);
        // Clip it by btr if needed
        if (rcnt > btf) {
            rcnt = btf;
        }
        // Forward the file data
        rcnt = (*func)(dbuf + ((UINT)fp->fptr % SS(fs)), rcnt);
        if (rcnt == 0) {
            ABORT(fs, FR_INT_ERR);
        }
    }

    LEAVE_FF(fs, FR_OK);
}

// FF_USE_FORWARD
#endif

#if !FF_FS_READONLY && FF_USE_MKFS
//-----------------------------------------------------------------------

// Create FAT/exFAT volume (with sub-functions)

//-----------------------------------------------------------------------

// Sectors per track for determination of drive CHS
static constexpr const uint32_t N_SEC_TRACK = 63;
// Alignment of partitions in GPT [byte] (>=128KB)
static constexpr const uint32_t GPT_ALIGN   = 0x100000;
// Number of GPT table size (>=128, sector aligned)
static constexpr const uint32_t GPT_ITEMS   = 128;

// Create partitions on the physical drive in format of MBR or GPT

static FRESULT                  create_partition(
                   // Physical drive number
  BYTE        drv,
  // Partition list
  const LBA_t plst[],
  // System ID for each partition (for only MBR)
  BYTE        sys,
  // Working buffer for a sector
  BYTE*       buf) {
    UINT  i, cy;
    LBA_t sz_drv;
    DWORD sz_drv32, nxt_alloc32, sz_part32;
    BYTE* pte;
    BYTE  hd, n_hd, sc, n_sc;

    // Get physical drive size

    if (disk_ioctl(drv, GET_SECTOR_COUNT, &sz_drv) != RES_OK) {
        return FR_DISK_ERR;
    }

#    if FF_LBA64
    // Create partitions in GPT format
    if (sz_drv >= FF_MIN_GPT) {
        WORD              ss;
        UINT              sz_ptbl, pi, si, ofs;
        DWORD             bcc, rnd, align;
        QWORD             nxt_alloc, sz_part, sz_pool, top_bpt;
        static const BYTE gpt_mbr[16]
          = { 0x00, 0x00, 0x02, 0x00, 0xEE, 0xFE, 0xFF, 0x00,
              0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };

#        if FF_MAX_SS != FF_MIN_SS
        // Get sector size
        if (disk_ioctl(drv, GET_SECTOR_SIZE, &ss) != RES_OK) {
            return FR_DISK_ERR;
        }
        if (ss > FF_MAX_SS || ss < FF_MIN_SS || (ss & (ss - 1))) {
            return FR_DISK_ERR;
        }
#        else
        ss = FF_MAX_SS;
#        endif
        // Random seed
        rnd       = (DWORD)sz_drv + GET_FATTIME();
        // Partition alignment for GPT [sector]
        align     = GPT_ALIGN / ss;
        // Size of partition table [sector]
        sz_ptbl   = GPT_ITEMS * SZ_GPTE / ss;
        // Backup partition table start sector
        top_bpt   = sz_drv - sz_ptbl - 1;
        // First allocatable sector
        nxt_alloc = 2 + sz_ptbl;
        // Size of allocatable area
        sz_pool   = top_bpt - nxt_alloc;
        bcc       = 0xFFFFFFFF;
        sz_part   = 1;
        // partition table index, size table index
        pi = si = 0;
        do {
            // Clean the buffer if needed
            if (pi * SZ_GPTE % ss == 0) {
                memset(buf, 0, ss);
            }
            // Is the size table not termintated?
            if (sz_part != 0) {
                // Align partition start
                nxt_alloc = (nxt_alloc + align - 1) & ((QWORD)0 - align);
                // Get a partition size
                sz_part   = plst[si++];
                // Is the size in percentage?
                if (sz_part <= 100) {
                    sz_part = sz_pool * sz_part / 100;
                    // Align partition end (only if in percentage)
                    sz_part = (sz_part + align - 1) & ((QWORD)0 - align);
                }
                // Clip the size at end of the pool
                if (nxt_alloc + sz_part > top_bpt) {
                    sz_part = (nxt_alloc < top_bpt) ? top_bpt - nxt_alloc : 0;
                }
            }
            // Add a partition?
            if (sz_part != 0) {
                ofs = pi * SZ_GPTE % ss;
                // Set partition GUID (Microsoft Basic Data)
                memcpy(buf + ofs + GPTE_PtGuid, GUID_MS_Basic, 16);
                // Set unique partition GUID
                rnd = make_rand(rnd, buf + ofs + GPTE_UpGuid, 16);
                // Set partition start sector
                st_qword(buf + ofs + GPTE_FstLba, nxt_alloc);
                // Set partition end sector
                st_qword(buf + ofs + GPTE_LstLba, nxt_alloc + sz_part - 1);
                // Next allocatable sector
                nxt_alloc += sz_part;
            }
            // Write the buffer if it is filled up
            if ((pi + 1) * SZ_GPTE % ss == 0) {
                // Calculate table check sum
                for (i = 0; i < ss; bcc = crc32(bcc, buf[i++]))
                    ;
                // Write to primary table
                if (disk_write(drv, buf, 2 + pi * SZ_GPTE / ss, 1) != RES_OK) {
                    return FR_DISK_ERR;
                }
                // Write to secondary table
                if (disk_write(drv, buf, top_bpt + pi * SZ_GPTE / ss, 1)
                    != RES_OK) {
                    return FR_DISK_ERR;
                }
            }
        } while (++pi < GPT_ITEMS);

        // Create primary GPT header

        memset(buf, 0, ss);
        // Signature, version (1.0) and size (92)
        memcpy(buf + GPTH_Sign,
               "EFI PART"
               "\0\0\1\0"
               "\x5C\0\0",
               16);
        // Table check sum
        st_dword(buf + GPTH_PtBcc, ~bcc);
        // LBA of this header
        st_qword(buf + GPTH_CurLba, 1);
        // LBA of secondary header
        st_qword(buf + GPTH_BakLba, sz_drv - 1);
        // LBA of first allocatable sector
        st_qword(buf + GPTH_FstLba, 2 + sz_ptbl);
        // LBA of last allocatable sector
        st_qword(buf + GPTH_LstLba, top_bpt - 1);
        // Size of a table entry
        st_dword(buf + GPTH_PteSize, SZ_GPTE);
        // Number of table entries
        st_dword(buf + GPTH_PtNum, GPT_ITEMS);
        // LBA of this table
        st_dword(buf + GPTH_PtOfs, 2);
        // Disk GUID
        rnd = make_rand(rnd, buf + GPTH_DskGuid, 16);
        // Calculate header check sum
        for (i = 0, bcc = 0xFFFFFFFF; i < 92; bcc = crc32(bcc, buf[i++]))
            ;
        // Header check sum
        st_dword(buf + GPTH_Bcc, ~bcc);
        if (disk_write(drv, buf, 1, 1) != RES_OK) {
            return FR_DISK_ERR;
        }

        // Create secondary GPT header

        // LBA of this header
        st_qword(buf + GPTH_CurLba, sz_drv - 1);
        // LBA of primary header
        st_qword(buf + GPTH_BakLba, 1);
        // LBA of this table
        st_qword(buf + GPTH_PtOfs, top_bpt);
        st_dword(buf + GPTH_Bcc, 0);
        // Calculate header check sum
        for (i = 0, bcc = 0xFFFFFFFF; i < 92; bcc = crc32(bcc, buf[i++]))
            ;
        // Header check sum
        st_dword(buf + GPTH_Bcc, ~bcc);
        if (disk_write(drv, buf, sz_drv - 1, 1) != RES_OK) {
            return FR_DISK_ERR;
        }

        // Create protective MBR

        memset(buf, 0, ss);
        // Create a GPT partition
        memcpy(buf + MBR_Table, gpt_mbr, 16);
        st_word(buf + BS_55AA, 0xAA55);
        if (disk_write(drv, buf, 0, 1) != RES_OK) {
            return FR_DISK_ERR;
        }
    }
    else
#    endif
    // Create partitions in MBR format
    {
        sz_drv32 = (DWORD)sz_drv;
        // Determine drive CHS without any consideration of the drive geometry
        n_sc     = N_SEC_TRACK;
        for (n_hd = 8; n_hd != 0 && sz_drv32 / n_hd / n_sc > 1024; n_hd *= 2)
            ;
        // Number of heads needs to be <256
        if (n_hd == 0) {
            n_hd = 255;
        }

        // Clear MBR
        memset(buf, 0, FF_MAX_SS);
        // Partition table in the MBR
        pte = buf + MBR_Table;
        for (i = 0, nxt_alloc32 = n_sc;
             i < 4 && nxt_alloc32 != 0 && nxt_alloc32 < sz_drv32;
             i++, nxt_alloc32 += sz_part32) {
            // Get partition size
            sz_part32 = (DWORD)plst[i];
            // Size in percentage?
            if (sz_part32 <= 100) {
                sz_part32
                  = (sz_part32 == 100) ? sz_drv32 : sz_drv32 / 100 * sz_part32;
            }
            // Clip at drive size
            if (nxt_alloc32 + sz_part32 > sz_drv32
                || nxt_alloc32 + sz_part32 < nxt_alloc32) {
                sz_part32 = sz_drv32 - nxt_alloc32;
            }
            // End of table or no sector to allocate?
            if (sz_part32 == 0) {
                break;
            }

            // Start LBA
            st_dword(pte + PTE_StLba, nxt_alloc32);
            // Number of sectors
            st_dword(pte + PTE_SizLba, sz_part32);
            // System type
            pte[PTE_System] = sys;

            // Start cylinder
            cy              = (UINT)(nxt_alloc32 / n_sc / n_hd);
            // Start head
            hd              = (BYTE)(nxt_alloc32 / n_sc % n_hd);
            // Start sector
            sc              = (BYTE)(nxt_alloc32 % n_sc + 1);
            pte[PTE_StHead] = hd;
            pte[PTE_StSec]  = (BYTE)((cy >> 2 & 0xC0) | sc);
            pte[PTE_StCyl]  = (BYTE)cy;

            // End cylinder
            cy = (UINT)((nxt_alloc32 + sz_part32 - 1) / n_sc / n_hd);
            // End head
            hd = (BYTE)((nxt_alloc32 + sz_part32 - 1) / n_sc % n_hd);
            // End sector
            sc = (BYTE)((nxt_alloc32 + sz_part32 - 1) % n_sc + 1);
            pte[PTE_EdHead]  = hd;
            pte[PTE_EdSec]   = (BYTE)((cy >> 2 & 0xC0) | sc);
            pte[PTE_EdCyl]   = (BYTE)cy;

            // Next entry
            pte             += SZ_PTE;
        }

        // MBR signature
        st_word(buf + BS_55AA, 0xAA55);
        // Write it to the MBR
        if (disk_write(drv, buf, 0, 1) != RES_OK) {
            return FR_DISK_ERR;
        }
    }

    return FR_OK;
}

FRESULT f_mkfs(const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len) {
    // Cluster size boundary for FAT volume (4Ks unit)
    static const WORD      cst[]   = { 1, 4, 16, 64, 256, 512, 0 };
    // Cluster size boundary for FAT32 volume (128Ks unit)
    static const WORD      cst32[] = { 1, 2, 4, 8, 16, 32, 0 };
    // Default parameter
    static const MKFS_PARM defopt  = { FM_ANY, 0, 0, 0, 0 };
    BYTE                   fsopt, fsty, sys, pdrv, ipart;
    BYTE*                  buf;
    BYTE*                  pte;
    // Sector size
    WORD                   ss;
    DWORD                  sz_buf, sz_blk, n_clst, pau, nsect, n, vsn;
    // Size of volume, Base LBA of volume, fat, data
    LBA_t                  sz_vol, b_vol, b_fat, b_data;
    LBA_t                  sect, lba[2];
    // Size of reserved, fat, dir, data, cluster
    DWORD                  sz_rsv, sz_fat, sz_dir, sz_au;
    // Index, Number of FATs and Number of roor dir entries
    UINT                   n_fat, n_root, i;
    int                    vol;
    DSTATUS                ds;
    FRESULT                res;

    // Check mounted drive and clear work area

    // Get target logical drive
    vol = get_ldnumber(&path);
    if (vol < 0) {
        return FR_INVALID_DRIVE;
    }
    // Clear the fs object if mounted
    if (FatFs[vol]) {
        FatFs[vol]->fs_type = 0;
    }
    // Hosting physical drive
    pdrv  = LD2PD(vol);
    // Hosting partition (0:create as new, 1..:existing partition)
    ipart = LD2PT(vol);

    // Initialize the hosting physical drive

    ds    = disk_initialize(pdrv);
    if (ds & STA_NOINIT) {
        return FR_NOT_READY;
    }
    if (ds & STA_PROTECT) {
        return FR_WRITE_PROTECTED;
    }

    // Get physical drive parameters (sz_drv, sz_blk and ss)

    // Use default parameter if it is not given
    if (!opt) {
        opt = &defopt;
    }
    sz_blk = opt->align;
    // Block size from the paramter or lower layer
    if (sz_blk == 0) {
        disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_blk);
    }
    // Use default if the block size is invalid
    if (sz_blk == 0 || sz_blk > 0x8000 || (sz_blk & (sz_blk - 1))) {
        sz_blk = 1;
    }
#    if FF_MAX_SS != FF_MIN_SS
    if (disk_ioctl(pdrv, GET_SECTOR_SIZE, &ss) != RES_OK) {
        return FR_DISK_ERR;
    }
    if (ss > FF_MAX_SS || ss < FF_MIN_SS || (ss & (ss - 1))) {
        return FR_DISK_ERR;
    }
#    else
    ss   = FF_MAX_SS;
#    endif

    // Options for FAT sub-type and FAT parameters

    fsopt  = opt->fmt & (FM_ANY | FM_SFD);
    n_fat  = (opt->n_fat >= 1 && opt->n_fat <= 2) ? opt->n_fat : 1;
    n_root = (opt->n_root >= 1 && opt->n_root <= 32768
              && (opt->n_root % (ss / SZDIRE)) == 0)
             ? opt->n_root
             : 512;
    sz_au
      = (opt->au_size <= 0x1000000 && (opt->au_size & (opt->au_size - 1)) == 0)
        ? opt->au_size
        : 0;
    // Byte --> Sector
    sz_au  /= ss;

    // Get working buffer

    // Size of working buffer [sector]
    sz_buf  = len / ss;
    if (sz_buf == 0) {
        return FR_NOT_ENOUGH_CORE;
    }
    // Working buffer
    buf = (BYTE*)work;
#    if FF_USE_LFN == 3
    // Use heap memory for working buffer
    if (!buf) {
        buf = ff_memalloc(sz_buf * ss);
    }
#    endif
    if (!buf) {
        return FR_NOT_ENOUGH_CORE;
    }

    // Determine where the volume to be located (b_vol, sz_vol)

    b_vol = sz_vol = 0;
    // Is the volume associated with any specific partition?
    if (FF_MULTI_PARTITION && ipart != 0) {
        // Get partition location from the existing partition table

        // Load MBR
        if (disk_read(pdrv, buf, 0, 1) != RES_OK) {
            LEAVE_MKFS(FR_DISK_ERR);
        }
        // Check if MBR is valid
        if (ld_word(buf + BS_55AA) != 0xAA55) {
            LEAVE_MKFS(FR_MKFS_ABORTED);
        }
#    if FF_LBA64
        // GPT protective MBR?
        if (buf[MBR_Table + PTE_System] == 0xEE) {
            DWORD n_ent, ofs;
            QWORD pt_lba;

            // Get the partition location from GPT

            // Load GPT header sector (next to MBR)
            if (disk_read(pdrv, buf, 1, 1) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            // Check if GPT header is valid
            if (!test_gpt_header(buf)) {
                LEAVE_MKFS(FR_MKFS_ABORTED);
            }
            // Number of entries
            n_ent  = ld_dword(buf + GPTH_PtNum);
            // Table start sector
            pt_lba = ld_qword(buf + GPTH_PtOfs);
            ofs = i = 0;
            // Find MS Basic partition with order of ipart
            while (n_ent) {
                // Get PT sector
                if (ofs == 0 && disk_read(pdrv, buf, pt_lba++, 1) != RES_OK) {
                    LEAVE_MKFS(FR_DISK_ERR);
                }
                // MS basic data partition?
                if (!memcmp(buf + ofs + GPTE_PtGuid, GUID_MS_Basic, 16)
                    && ++i == ipart) {
                    b_vol  = ld_qword(buf + ofs + GPTE_FstLba);
                    sz_vol = ld_qword(buf + ofs + GPTE_LstLba) - b_vol + 1;
                    break;
                }
                // Next entry
                n_ent--;
                ofs = (ofs + SZ_GPTE) % ss;
            }
            // Partition not found
            if (n_ent == 0) {
                LEAVE_MKFS(FR_MKFS_ABORTED);
            }
            // Partitioning is in GPT
            fsopt |= 0x80;
        }
        else
#    endif
        // Get the partition location from MBR partition table
        {
            pte = buf + (MBR_Table + (ipart - 1) * SZ_PTE);
            // No partition?
            if (ipart > 4 || pte[PTE_System] == 0) {
                LEAVE_MKFS(FR_MKFS_ABORTED);
            }
            // Get volume start sector
            b_vol  = ld_dword(pte + PTE_StLba);
            // Get volume size
            sz_vol = ld_dword(pte + PTE_SizLba);
        }
        // The volume is associated with a physical drive
    }
    else {
        if (disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_vol) != RES_OK) {
            LEAVE_MKFS(FR_DISK_ERR);
        }
        // To be partitioned?
        if (!(fsopt & FM_SFD)) {
            // Create a single-partition on the drive in this function

#    if FF_LBA64
            // Which partition type to create, MBR or GPT?
            if (sz_vol >= FF_MIN_GPT) {
                // Partitioning is in GPT
                fsopt  |= 0x80;
                // Estimated partition offset and size
                b_vol   = GPT_ALIGN / ss;
                sz_vol -= b_vol + GPT_ITEMS * SZ_GPTE / ss + 1;
            }
            else
#    endif
            // Partitioning is in MBR
            {
                if (sz_vol > N_SEC_TRACK) {
                    // Estimated partition offset and size
                    b_vol   = N_SEC_TRACK;
                    sz_vol -= b_vol;
                }
            }
        }
    }
    // Check if volume size is >=128s
    if (sz_vol < 128) {
        LEAVE_MKFS(FR_MKFS_ABORTED);
    }

    // Now start to create an FAT volume at b_vol and sz_vol

    // Pre-determine the FAT type
    do {
        // exFAT possible?
        if (FF_FS_EXFAT && (fsopt & FM_EXFAT)) {
            // exFAT only, vol >= 64MS or sz_au > 128S ?
            if ((fsopt & FM_ANY) == FM_EXFAT || sz_vol >= 0x4000000
                || sz_au > 128) {
                fsty = FS_EXFAT;
                break;
            }
        }
#    if FF_LBA64
        // Too large volume for FAT/FAT32
        if (sz_vol >= 0x100000000) {
            LEAVE_MKFS(FR_MKFS_ABORTED);
        }
#    endif
        // Invalid AU for FAT/FAT32?
        if (sz_au > 128) {
            sz_au = 128;
        }
        // FAT32 possible?
        if (fsopt & FM_FAT32) {
            // no-FAT?
            if (!(fsopt & FM_FAT)) {
                fsty = FS_FAT32;
                break;
            }
        }
        // no-FAT?
        if (!(fsopt & FM_FAT)) {
            LEAVE_MKFS(FR_INVALID_PARAMETER);
        }
        fsty = FS_FAT16;
    } while (0);

    // VSN generated from current time and partitiion size
    vsn = (DWORD)sz_vol + GET_FATTIME();

#    if FF_FS_EXFAT
    // Create an exFAT volume
    if (fsty == FS_EXFAT) {
        DWORD szb_bit, szb_case, sum, nbit, clu, clen[3];
        WCHAR ch, si;
        UINT  j, st;

        // Too small volume for exFAT?
        if (sz_vol < 0x1000) {
            LEAVE_MKFS(FR_MKFS_ABORTED);
        }
#        if FF_USE_TRIM
        // Inform storage device that the volume area may be erased
        lba[0] = b_vol;
        lba[1] = b_vol + sz_vol - 1;
        disk_ioctl(pdrv, CTRL_TRIM, lba);
#        endif
        // Determine FAT location, data location and number of clusters

        // AU auto-selection
        if (sz_au == 0) {
            sz_au = 8;
            // >= 512Ks
            if (sz_vol >= 0x80000) {
                sz_au = 64;
            }
            // >= 64Ms
            if (sz_vol >= 0x4000000) {
                sz_au = 256;
            }
        }
        // FAT start at offset 32
        b_fat  = b_vol + 32;
        // Number of FAT sectors
        sz_fat = (DWORD)((sz_vol / sz_au + 2) * 4 + ss - 1) / ss;
        // Align data area to the erase block boundary
        b_data = (b_fat + sz_fat + sz_blk - 1) & ~((LBA_t)sz_blk - 1);
        // Too small volume?
        if (b_data - b_vol >= sz_vol / 2) {
            LEAVE_MKFS(FR_MKFS_ABORTED);
        }
        // Number of clusters
        n_clst = (DWORD)((sz_vol - (b_data - b_vol)) / sz_au);
        // Too few clusters?
        if (n_clst < 16) {
            LEAVE_MKFS(FR_MKFS_ABORTED);
        }
        // Too many clusters?
        if (n_clst > MAX_EXFAT) {
            LEAVE_MKFS(FR_MKFS_ABORTED);
        }

        // Size of allocation bitmap
        szb_bit  = (n_clst + 7) / 8;
        // Number of allocation bitmap clusters
        clen[0]  = (szb_bit + sz_au * ss - 1) / (sz_au * ss);

        // Create a compressed up-case table

        // Table start sector
        sect     = b_data + sz_au * clen[0];
        // Table checksum to be stored in the 82 entry
        sum      = 0;
        st       = 0;
        si       = 0;
        i        = 0;
        j        = 0;
        szb_case = 0;
        do {
            switch (st) {
                case 0:
                    // Get an up-case char
                    ch = (WCHAR)ff_wtoupper(si);
                    if (ch != si) {
                        // Store the up-case char if exist
                        si++;
                        break;
                    }
                    // Get run length of no-case block
                    for (j = 1;
                         (WCHAR)(si + j)
                         && (WCHAR)(si + j) == ff_wtoupper((WCHAR)(si + j));
                         j++)
                        ;
                    if (j >= 128) {
                        // Compress the no-case block if run is >= 128 chars
                        ch = 0xFFFF;
                        st = 2;
                        break;
                    }
                    // Do not compress short run
                    st = 1;
                    // FALLTHROUGH

                case 1:
                    // Fill the short run
                    ch = si++;
                    if (--j == 0) {
                        st = 0;
                    }
                    break;

                default:
                    // Number of chars to skip
                    ch  = (WCHAR)j;
                    si += (WCHAR)j;
                    st  = 0;
            }
            // Put it into the write buffer
            sum       = xsum32(buf[i + 0] = (BYTE)ch, sum);
            sum       = xsum32(buf[i + 1] = (BYTE)(ch >> 8), sum);
            i        += 2;
            szb_case += 2;
            // Write buffered data when buffer full or end of process
            if (si == 0 || i == sz_buf * ss) {
                n = (i + ss - 1) / ss;
                if (disk_write(pdrv, buf, sect, n) != RES_OK) {
                    LEAVE_MKFS(FR_DISK_ERR);
                }
                sect += n;
                i     = 0;
            }
        } while (si);
        // Number of up-case table clusters
        clen[1] = (szb_case + sz_au * ss - 1) / (sz_au * ss);
        // Number of root dir clusters
        clen[2] = 1;

        // Initialize the allocation bitmap

        // Start of bitmap and number of bitmap sectors
        sect    = b_data;
        nsect   = (szb_bit + ss - 1) / ss;
        // Number of clusters in-use by system (bitmap, up-case and root-dir)
        nbit    = clen[0] + clen[1] + clen[2];
        do {
            // Initialize bitmap buffer
            memset(buf, 0, sz_buf * ss);
            // Mark used clusters
            for (i           = 0; nbit != 0 && i / 8 < sz_buf * ss;
                 buf[i / 8] |= 1 << (i % 8), i++, nbit--)
                ;
            // Write the buffered data
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, n) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            sect  += n;
            nsect -= n;
        } while (nsect);

        // Initialize the FAT

        // Start of FAT and number of FAT sectors
        sect  = b_fat;
        nsect = sz_fat;
        j = nbit = clu = 0;
        do {
            // Clear work area and reset write offset
            memset(buf, 0, sz_buf * ss);
            i = 0;
            // Initialize FAT [0] and FAT[1]
            if (clu == 0) {
                st_dword(buf + i, 0xFFFFFFF8);
                i += 4;
                clu++;
                st_dword(buf + i, 0xFFFFFFFF);
                i += 4;
                clu++;
            }
            // Create chains of bitmap, up-case and root dir
            do {
                // Create a chain
                while (nbit != 0 && i < sz_buf * ss) {
                    st_dword(buf + i, (nbit > 1) ? clu + 1 : 0xFFFFFFFF);
                    i += 4;
                    clu++;
                    nbit--;
                }
                // Get next chain length
                if (nbit == 0 && j < 3) {
                    nbit = clen[j++];
                }
            } while (nbit != 0 && i < sz_buf * ss);
            // Write the buffered data
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, n) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            sect  += n;
            nsect -= n;
        } while (nsect);

        // Initialize the root directory

        memset(buf, 0, sz_buf * ss);
        // Volume label entry (no label)
        buf[SZDIRE * 0 + 0] = ET_VLABEL;
        // Bitmap entry
        buf[SZDIRE * 1 + 0] = ET_BITMAP;
        // cluster
        st_dword(buf + SZDIRE * 1 + 20, 2);
        // size
        st_dword(buf + SZDIRE * 1 + 24, szb_bit);
        // Up-case table entry
        buf[SZDIRE * 2 + 0] = ET_UPCASE;
        // sum
        st_dword(buf + SZDIRE * 2 + 4, sum);
        // cluster
        st_dword(buf + SZDIRE * 2 + 20, 2 + clen[0]);
        // size
        st_dword(buf + SZDIRE * 2 + 24, szb_case);
        // Start of the root directory and number of sectors
        sect  = b_data + sz_au * (clen[0] + clen[1]);
        nsect = sz_au;
        // Fill root directory sectors
        do {
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, n) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            // Rest of entries are filled with zero
            memset(buf, 0, ss);
            sect  += n;
            nsect -= n;
        } while (nsect);

        // Create two set of the exFAT VBR blocks

        sect = b_vol;
        for (n = 0; n < 2; n++) {
            // Main record (+0)

            memset(buf, 0, ss);
            // Boot jump code (x86), OEM name
            memcpy(buf + BS_JmpBoot,
                   "\xEB\x76\x90"
                   "EXFAT   ",
                   11);
            // Volume offset in the physical drive [sector]
            st_qword(buf + BPB_VolOfsEx, b_vol);
            // Volume size [sector]
            st_qword(buf + BPB_TotSecEx, sz_vol);
            // FAT offset [sector]
            st_dword(buf + BPB_FatOfsEx, (DWORD)(b_fat - b_vol));
            // FAT size [sector]
            st_dword(buf + BPB_FatSzEx, sz_fat);
            // Data offset [sector]
            st_dword(buf + BPB_DataOfsEx, (DWORD)(b_data - b_vol));
            // Number of clusters
            st_dword(buf + BPB_NumClusEx, n_clst);
            // Root dir cluster #
            st_dword(buf + BPB_RootClusEx, 2 + clen[0] + clen[1]);
            // VSN
            st_dword(buf + BPB_VolIDEx, vsn);
            // Filesystem version (1.00)
            st_word(buf + BPB_FSVerEx, 0x100);
            // Log2 of sector size [byte]
            for (buf[BPB_BytsPerSecEx] = 0, i = ss; i >>= 1;
                 buf[BPB_BytsPerSecEx]++)
                ;
            // Log2 of cluster size [sector]
            for (buf[BPB_SecPerClusEx] = 0, i = sz_au; i >>= 1;
                 buf[BPB_SecPerClusEx]++)
                ;
            // Number of FATs
            buf[BPB_NumFATsEx] = 1;
            // Drive number (for int13)
            buf[BPB_DrvNumEx]  = 0x80;
            // Boot code (x86)
            st_word(buf + BS_BootCodeEx, 0xFEEB);
            // Signature (placed here regardless of sector size)
            st_word(buf + BS_55AA, 0xAA55);
            // VBR checksum
            for (i = sum = 0; i < ss; i++) {
                if (i != BPB_VolFlagEx && i != BPB_VolFlagEx + 1
                    && i != BPB_PercInUseEx) {
                    sum = xsum32(buf[i], sum);
                }
            }
            if (disk_write(pdrv, buf, sect++, 1) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            // Extended bootstrap record (+1..+8)

            memset(buf, 0, ss);
            // Signature (placed at end of sector)
            st_word(buf + ss - 2, 0xAA55);
            for (j = 1; j < 9; j++) {
                // VBR checksum
                for (i = 0; i < ss; sum = xsum32(buf[i++], sum))
                    ;
                if (disk_write(pdrv, buf, sect++, 1) != RES_OK) {
                    LEAVE_MKFS(FR_DISK_ERR);
                }
            }
            // OEM/Reserved record (+9..+10)

            memset(buf, 0, ss);
            for (; j < 11; j++) {
                // VBR checksum
                for (i = 0; i < ss; sum = xsum32(buf[i++], sum))
                    ;
                if (disk_write(pdrv, buf, sect++, 1) != RES_OK) {
                    LEAVE_MKFS(FR_DISK_ERR);
                }
            }
            // Sum record (+11)

            // Fill with checksum value
            for (i = 0; i < ss; i += 4) {
                st_dword(buf + i, sum);
            }
            if (disk_write(pdrv, buf, sect++, 1) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
        }
    }
    else
    // FF_FS_EXFAT
#    endif
    // Create an FAT/FAT32 volume
    {
        do {
            pau = sz_au;
            // Pre-determine number of clusters and FAT sub-type

            // FAT32 volume
            if (fsty == FS_FAT32) {
                // AU auto-selection
                if (pau == 0) {
                    // Volume size in unit of 128KS
                    n = (DWORD)sz_vol / 0x20000;
                    // Get from table
                    for (i = 0, pau   = 1; cst32[i] && cst32[i] <= n;
                         i++, pau   <<= 1)
                        ;
                }
                // Number of clusters
                n_clst = (DWORD)sz_vol / pau;
                // FAT size [sector]
                sz_fat = (n_clst * 4 + 8 + ss - 1) / ss;
                // Number of reserved sectors
                sz_rsv = 32;
                // No static directory
                sz_dir = 0;
                if (n_clst <= MAX_FAT16 || n_clst > MAX_FAT32) {
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
                // FAT volume
            }
            else {
                // au auto-selection
                if (pau == 0) {
                    // Volume size in unit of 4KS
                    n = (DWORD)sz_vol / 0x1000;
                    // Get from table
                    for (i = 0, pau = 1; cst[i] && cst[i] <= n; i++, pau <<= 1)
                        ;
                }
                n_clst = (DWORD)sz_vol / pau;
                if (n_clst > MAX_FAT12) {
                    // FAT size [byte]
                    n = n_clst * 2 + 4;
                }
                else {
                    fsty = FS_FAT12;
                    // FAT size [byte]
                    n    = (n_clst * 3 + 1) / 2 + 3;
                }
                // FAT size [sector]
                sz_fat = (n + ss - 1) / ss;
                // Number of reserved sectors
                sz_rsv = 1;
                // Root dir size [sector]
                sz_dir = (DWORD)n_root * SZDIRE / ss;
            }
            // FAT base
            b_fat  = b_vol + sz_rsv;
            // Data base
            b_data = b_fat + sz_fat * n_fat + sz_dir;

            // Align data area to erase block boundary (for flash memory media)

            // Sectors to next nearest from current data base
            n      = (DWORD)(((b_data + sz_blk - 1) & ~(sz_blk - 1)) - b_data);
            // FAT32: Move FAT
            if (fsty == FS_FAT32) {
                sz_rsv += n;
                b_fat  += n;
                // FAT: Expand FAT
            }
            else {
                // Adjust fractional error if needed
                if (n % n_fat) {
                    n--;
                    sz_rsv++;
                    b_fat++;
                }
                sz_fat += n / n_fat;
            }

            // Determine number of clusters and final check of validity of the
            // FAT sub-type

            // Too small volume?
            if (sz_vol < b_data + pau * 16 - b_vol) {
                LEAVE_MKFS(FR_MKFS_ABORTED);
            }
            n_clst = ((DWORD)sz_vol - sz_rsv - sz_fat * n_fat - sz_dir) / pau;
            if (fsty == FS_FAT32) {
                // Too few clusters for FAT32?
                if (n_clst <= MAX_FAT16) {
                    // Adjust cluster size and retry
                    if (sz_au == 0 && (sz_au = pau / 2) != 0) {
                        continue;
                    }
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
            }
            if (fsty == FS_FAT16) {
                // Too many clusters for FAT16
                if (n_clst > MAX_FAT16) {
                    if (sz_au == 0 && (pau * 2) <= 64) {
                        // Adjust cluster size and retry
                        sz_au = pau * 2;
                        continue;
                    }
                    if ((fsopt & FM_FAT32)) {
                        // Switch type to FAT32 and retry
                        fsty = FS_FAT32;
                        continue;
                    }
                    // Adjust cluster size and retry
                    if (sz_au == 0 && (sz_au = pau * 2) <= 128) {
                        continue;
                    }
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
                // Too few clusters for FAT16
                if (n_clst <= MAX_FAT12) {
                    // Adjust cluster size and retry
                    if (sz_au == 0 && (sz_au = pau * 2) <= 128) {
                        continue;
                    }
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
            }
            // Too many clusters for FAT12
            if (fsty == FS_FAT12 && n_clst > MAX_FAT12) {
                LEAVE_MKFS(FR_MKFS_ABORTED);
            }

            // Ok, it is the valid cluster configuration

            break;
        } while (1);

#    if FF_USE_TRIM
        // Inform storage device that the volume area may be erased
        lba[0] = b_vol;
        lba[1] = b_vol + sz_vol - 1;
        disk_ioctl(pdrv, CTRL_TRIM, lba);
#    endif
        // Create FAT VBR

        memset(buf, 0, ss);
        // Boot jump code (x86), OEM name
        memcpy(buf + BS_JmpBoot,
               "\xEB\xFE\x90"
               "MSDOS5.0",
               11);
        // Sector size [byte]
        st_word(buf + BPB_BytsPerSec, ss);
        // Cluster size [sector]
        buf[BPB_SecPerClus] = (BYTE)pau;
        // Size of reserved area
        st_word(buf + BPB_RsvdSecCnt, (WORD)sz_rsv);
        // Number of FATs
        buf[BPB_NumFATs] = (BYTE)n_fat;
        // Number of root directory entries
        st_word(buf + BPB_RootEntCnt, (WORD)((fsty == FS_FAT32) ? 0 : n_root));
        if (sz_vol < 0x10000) {
            // Volume size in 16-bit LBA
            st_word(buf + BPB_TotSec16, (WORD)sz_vol);
        }
        else {
            // Volume size in 32-bit LBA
            st_dword(buf + BPB_TotSec32, (DWORD)sz_vol);
        }
        // Media descriptor byte
        buf[BPB_Media] = 0xF8;
        // Number of sectors per track (for int13)
        st_word(buf + BPB_SecPerTrk, 63);
        // Number of heads (for int13)
        st_word(buf + BPB_NumHeads, 255);
        // Volume offset in the physical drive [sector]
        st_dword(buf + BPB_HiddSec, (DWORD)b_vol);
        if (fsty == FS_FAT32) {
            // VSN
            st_dword(buf + BS_VolID32, vsn);
            // FAT size [sector]
            st_dword(buf + BPB_FATSz32, sz_fat);
            // Root directory cluster # (2)
            st_dword(buf + BPB_RootClus32, 2);
            // Offset of FSINFO sector (VBR + 1)
            st_word(buf + BPB_FSInfo32, 1);
            // Offset of backup VBR (VBR + 6)
            st_word(buf + BPB_BkBootSec32, 6);
            // Drive number (for int13)
            buf[BS_DrvNum32]  = 0x80;
            // Extended boot signature
            buf[BS_BootSig32] = 0x29;
            // Volume label, FAT signature
            memcpy(buf + BS_VolLab32,
                   "NO NAME    "
                   "FAT32   ",
                   19);
        }
        else {
            // VSN
            st_dword(buf + BS_VolID, vsn);
            // FAT size [sector]
            st_word(buf + BPB_FATSz16, (WORD)sz_fat);
            // Drive number (for int13)
            buf[BS_DrvNum]  = 0x80;
            // Extended boot signature
            buf[BS_BootSig] = 0x29;
            // Volume label, FAT signature
            memcpy(buf + BS_VolLab,
                   "NO NAME    "
                   "FAT     ",
                   19);
        }
        // Signature (offset is fixed here regardless of sector size)
        st_word(buf + BS_55AA, 0xAA55);
        // Write it to the VBR sector
        if (disk_write(pdrv, buf, b_vol, 1) != RES_OK) {
            LEAVE_MKFS(FR_DISK_ERR);
        }

        // Create FSINFO record if needed

        if (fsty == FS_FAT32) {
            // Write backup VBR (VBR + 6)
            disk_write(pdrv, buf, b_vol + 6, 1);
            memset(buf, 0, ss);
            st_dword(buf + FSI_LeadSig, 0x41615252);
            st_dword(buf + FSI_StrucSig, 0x61417272);
            // Number of free clusters
            st_dword(buf + FSI_Free_Count, n_clst - 1);
            // Last allocated cluster#
            st_dword(buf + FSI_Nxt_Free, 2);
            st_word(buf + BS_55AA, 0xAA55);
            // Write backup FSINFO (VBR + 7)
            disk_write(pdrv, buf, b_vol + 7, 1);
            // Write original FSINFO (VBR + 1)
            disk_write(pdrv, buf, b_vol + 1, 1);
        }

        // Initialize FAT area

        memset(buf, 0, sz_buf * ss);
        // FAT start sector
        sect = b_fat;
        // Initialize FATs each
        for (i = 0; i < n_fat; i++) {
            if (fsty == FS_FAT32) {
                // FAT[0]
                st_dword(buf + 0, 0xFFFFFFF8);
                // FAT[1]
                st_dword(buf + 4, 0xFFFFFFFF);
                // FAT[2] (root directory)
                st_dword(buf + 8, 0x0FFFFFFF);
            }
            else {
                // FAT[0] and FAT[1]
                st_dword(buf + 0, (fsty == FS_FAT12) ? 0xFFFFF8 : 0xFFFFFFF8);
            }
            // Number of FAT sectors
            nsect = sz_fat;
            // Fill FAT sectors
            do {
                n = (nsect > sz_buf) ? sz_buf : nsect;
                if (disk_write(pdrv, buf, sect, (UINT)n) != RES_OK) {
                    LEAVE_MKFS(FR_DISK_ERR);
                }
                // Rest of FAT all are cleared
                memset(buf, 0, ss);
                sect  += n;
                nsect -= n;
            } while (nsect);
        }

        // Initialize root directory (fill with zero)

        // Number of root directory sectors
        nsect = (fsty == FS_FAT32) ? pau : sz_dir;
        do {
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, (UINT)n) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            sect  += n;
            nsect -= n;
        } while (nsect);
    }

    // A FAT volume has been created here

    // Determine system ID in the MBR partition table

    if (FF_FS_EXFAT && fsty == FS_EXFAT) {
        // exFAT
        sys = 0x07;
    }
    else if (fsty == FS_FAT32) {
        // FAT32X
        sys = 0x0C;
    }
    else if (sz_vol >= 0x10000) {
        // FAT12/16 (large)
        sys = 0x06;
    }
    else if (fsty == FS_FAT16) {
        // FAT16
        sys = 0x04;
    }
    else {
        // FAT12
        sys = 0x01;
    }

    // Update partition information

    // Volume is in the existing partition
    if (FF_MULTI_PARTITION && ipart != 0) {
        // Is the partition in MBR?
        if (!FF_LBA64 || !(fsopt & 0x80)) {
            // Update system ID in the partition table

            // Read the MBR
            if (disk_read(pdrv, buf, 0, 1) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
            // Set system ID
            buf[MBR_Table + (ipart - 1) * SZ_PTE + PTE_System] = sys;
            // Write it back to the MBR
            if (disk_write(pdrv, buf, 0, 1) != RES_OK) {
                LEAVE_MKFS(FR_DISK_ERR);
            }
        }
        // Volume as a new single partition
    }
    else {
        // Create partition table if not in SFD format
        if (!(fsopt & FM_SFD)) {
            lba[0] = sz_vol;
            lba[1] = 0;
            res    = create_partition(pdrv, lba, sys, buf);
            if (res != FR_OK) {
                LEAVE_MKFS(res);
            }
        }
    }

    if (disk_ioctl(pdrv, CTRL_SYNC, 0) != RES_OK) {
        LEAVE_MKFS(FR_DISK_ERR);
    }

    LEAVE_MKFS(FR_OK);
}

#    if FF_MULTI_PARTITION
//-----------------------------------------------------------------------

// Create Partition Table on the Physical Drive

//-----------------------------------------------------------------------

FRESULT f_fdisk(BYTE pdrv, const LBA_t ptbl[], void* work) {
    BYTE*   buf = (BYTE*)work;
    DSTATUS stat;
    FRESULT res;

    // Initialize the physical drive

    stat = disk_initialize(pdrv);
    if (stat & STA_NOINIT) {
        return FR_NOT_READY;
    }
    if (stat & STA_PROTECT) {
        return FR_WRITE_PROTECTED;
    }

#        if FF_USE_LFN == 3
    // Use heap memory for working buffer
    if (!buf) {
        buf = ff_memalloc(FF_MAX_SS);
    }
#        endif
    if (!buf) {
        return FR_NOT_ENOUGH_CORE;
    }

    // Create partitions (system ID is temporary setting and determined by
    // f_mkfs)
    res = create_partition(pdrv, ptbl, 0x07, buf);

    LEAVE_MKFS(res);
}

// FF_MULTI_PARTITION
#    endif
// !FF_FS_READONLY && FF_USE_MKFS
#endif

#if FF_USE_STRFUNC
#    if FF_USE_LFN && FF_LFN_UNICODE \
      && (FF_STRF_ENCODE < 0 || FF_STRF_ENCODE > 3)
#        error Wrong FF_STRF_ENCODE setting
#    endif
//-----------------------------------------------------------------------

// Get a String from the File

//-----------------------------------------------------------------------

TCHAR* f_gets(TCHAR* buff, int len, FIL* fp) {
    int    nc = 0;
    TCHAR* p  = buff;
    BYTE   s[4];
    UINT   rc;
    DWORD  dc;
#    if FF_USE_LFN && FF_LFN_UNICODE && FF_STRF_ENCODE <= 2
    WCHAR wc;
#    endif
#    if FF_USE_LFN && FF_LFN_UNICODE && FF_STRF_ENCODE == 3
    UINT ct;
#    endif

// With code conversion (Unicode API)
#    if FF_USE_LFN && FF_LFN_UNICODE
    // Make a room for the character and terminator

    if (FF_LFN_UNICODE == 1) {
        len -= (FF_STRF_ENCODE == 0) ? 1 : 2;
    }
    if (FF_LFN_UNICODE == 2) {
        len -= (FF_STRF_ENCODE == 0) ? 3 : 4;
    }
    if (FF_LFN_UNICODE == 3) {
        len -= 1;
    }
    while (nc < len) {
        // Read a character in ANSI/OEM
#        if FF_STRF_ENCODE == 0
        // Get a code unit
        f_read(fp, s, 1, &rc);
        // EOF?
        if (rc != 1) {
            break;
        }
        wc = s[0];
        // DBC 1st byte?
        if (dbc_1st((BYTE)wc)) {
            // Get 2nd byte
            f_read(fp, s, 1, &rc);
            // Wrong code?
            if (rc != 1 || !dbc_2nd(s[0])) {
                continue;
            }
            wc = wc << 8 | s[0];
        }
        // Convert ANSI/OEM into Unicode
        dc = ff_oem2uni(wc, CODEPAGE);
        // Conversion error?
        if (dc == 0) {
            continue;
        }
        // Read a character in UTF-16LE/BE
#        elif FF_STRF_ENCODE == 1 || FF_STRF_ENCODE == 2
        // Get a code unit
        f_read(fp, s, 2, &rc);
        // EOF?
        if (rc != 2) {
            break;
        }
        dc = (FF_STRF_ENCODE == 1) ? ld_word(s) : s[0] << 8 | s[1];
        // Broken surrogate pair?
        if (IsSurrogateL(dc)) {
            continue;
        }
        // High surrogate?
        if (IsSurrogateH(dc)) {
            // Get low surrogate
            f_read(fp, s, 2, &rc);
            // EOF?
            if (rc != 2) {
                break;
            }
            wc = (FF_STRF_ENCODE == 1) ? ld_word(s) : s[0] << 8 | s[1];
            // Broken surrogate pair?
            if (!IsSurrogateL(wc)) {
                continue;
            }
            // Merge surrogate pair
            dc = ((dc & 0x3FF) + 0x40) << 10 | (wc & 0x3FF);
        }
        // Read a character in UTF-8
#        else
        // Get a code unit
        f_read(fp, s, 1, &rc);
        // EOF?
        if (rc != 1) {
            break;
        }
        dc = s[0];
        // Multi-byte sequence?
        if (dc >= 0x80) {
            ct = 0;
            // 2-byte sequence?
            if ((dc & 0xE0) == 0xC0) {
                dc &= 0x1F;
                ct  = 1;
            }
            // 3-byte sequence?
            if ((dc & 0xF0) == 0xE0) {
                dc &= 0x0F;
                ct  = 2;
            }
            // 4-byte sequence?
            if ((dc & 0xF8) == 0xF0) {
                dc &= 0x07;
                ct  = 3;
            }
            if (ct == 0) {
                continue;
            }
            // Get trailing bytes
            f_read(fp, s, ct, &rc);
            if (rc != ct) {
                break;
            }
            rc = 0;
            // Merge the byte sequence
            do {
                if ((s[rc] & 0xC0) != 0x80) {
                    break;
                }
                dc = dc << 6 | (s[rc] & 0x3F);
            } while (++rc < ct);
            // Wrong encoding?
            if (rc != ct || dc < 0x80 || IsSurrogate(dc) || dc >= 0x110000) {
                continue;
            }
        }
#        endif
        // A code point is avaialble in dc to be output

        // Strip \r off if needed
        if (FF_USE_STRFUNC == 2 && dc == '\r') {
            continue;
        }
        // Output it in UTF-16/32 encoding
#        if FF_LFN_UNICODE == 1 || FF_LFN_UNICODE == 3
        // Out of BMP at UTF-16?
        if (FF_LFN_UNICODE == 1 && dc >= 0x10000) {
            // Make and output high surrogate
            *p++ = (TCHAR)(0xD800 | ((dc >> 10) - 0x40));
            nc++;
            // Make low surrogate
            dc = 0xDC00 | (dc & 0x3FF);
        }
        *p++ = (TCHAR)dc;
        nc++;
        // End of line?
        if (dc == '\n') {
            break;
        }
        // Output it in UTF-8 encoding
#        elif FF_LFN_UNICODE == 2
        // Single byte?
        if (dc < 0x80) {
            *p++ = (TCHAR)dc;
            nc++;
            // End of line?
            if (dc == '\n') {
                break;
            }
            // 2-byte sequence?
        }
        else if (dc < 0x800) {
            *p++  = (TCHAR)(0xC0 | (dc >> 6 & 0x1F));
            *p++  = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
            nc   += 2;
            // 3-byte sequence?
        }
        else if (dc < 0x10000) {
            *p++  = (TCHAR)(0xE0 | (dc >> 12 & 0x0F));
            *p++  = (TCHAR)(0x80 | (dc >> 6 & 0x3F));
            *p++  = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
            nc   += 3;
            // 4-byte sequence
        }
        else {
            *p++  = (TCHAR)(0xF0 | (dc >> 18 & 0x07));
            *p++  = (TCHAR)(0x80 | (dc >> 12 & 0x3F));
            *p++  = (TCHAR)(0x80 | (dc >> 6 & 0x3F));
            *p++  = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
            nc   += 4;
        }
#        endif
    }

// Byte-by-byte read without any conversion (ANSI/OEM API)
#    else
    // Make a room for the terminator
    len -= 1;
    while (nc < len) {
        // Get a byte
        f_read(fp, s, 1, &rc);
        // EOF?
        if (rc != 1) {
            break;
        }
        dc = s[0];
        if (FF_USE_STRFUNC == 2 && dc == '\r') {
            continue;
        }
        *p++ = (TCHAR)dc;
        nc++;
        if (dc == '\n') {
            break;
        }
    }
#    endif

    // Terminate the string
    *p = 0;
    // When no data read due to EOF or error, return with error.
    return nc ? buff : 0;
}

#    if !FF_FS_READONLY
#        include <stdarg.h>
static constexpr const uint32_t SZ_PUTC_BUF = 64;
static constexpr const uint32_t SZ_NUM_BUF  = 32;

//-----------------------------------------------------------------------

// Put a Character to the File (with sub-functions)

//-----------------------------------------------------------------------

// Output buffer and work area

struct putbuff {
    // Ptr to the writing file
    FIL* fp;
    // Write index of buf[] (-1:error), number of encoding units written
    int  idx, nchr;
#        if FF_USE_LFN && FF_LFN_UNICODE == 1
    WCHAR hs;
#        elif FF_USE_LFN && FF_LFN_UNICODE == 2
    BYTE bs[4];
    UINT wi, ct;
#        endif
    // Write buffer
    BYTE buf[SZ_PUTC_BUF];
};

// Buffered file write with code conversion

static void putc_bfd(putbuff* pb, TCHAR c) {
    UINT n;
    int  i, nc;
#        if FF_USE_LFN && FF_LFN_UNICODE
    WCHAR hs, wc;
#            if FF_LFN_UNICODE == 2
    DWORD        dc;
    const TCHAR* tp;
#            endif
#        endif

    // LF -> CRLF conversion
    if (FF_USE_STRFUNC == 2 && c == '\n') {
        putc_bfd(pb, '\r');
    }

    // Write index of pb->buf[]
    i = pb->idx;
    // In write error?
    if (i < 0) {
        return;
    }
    // Write unit counter
    nc = pb->nchr;

#        if FF_USE_LFN && FF_LFN_UNICODE
// UTF-16 input
#            if FF_LFN_UNICODE == 1
    // Is this a high-surrogate?
    if (IsSurrogateH(c)) {
        // Save it for next
        pb->hs = c;
        return;
    }
    hs     = pb->hs;
    pb->hs = 0;
    // Is there a leading high-surrogate?
    if (hs != 0) {
        // Discard high-surrogate if not a surrogate pair
        if (!IsSurrogateL(c)) {
            hs = 0;
        }
    }
    else {
        // Discard stray low-surrogate
        if (IsSurrogateL(c)) {
            return;
        }
    }
    wc = c;
    // UTF-8 input
#            elif FF_LFN_UNICODE == 2
    for (;;) {
        // Out of multi-byte sequence?
        if (pb->ct == 0) {
            // Save 1st byte
            pb->bs[pb->wi = 0] = (BYTE)c;
            // Single byte code?
            if ((BYTE)c < 0x80) {
                break;
            }
            // 2-byte sequence?
            if (((BYTE)c & 0xE0) == 0xC0) {
                pb->ct = 1;
            }
            // 3-byte sequence?
            if (((BYTE)c & 0xF0) == 0xE0) {
                pb->ct = 2;
            }
            // 4-byte sequence?
            if (((BYTE)c & 0xF8) == 0xF0) {
                pb->ct = 3;
            }
            // Wrong leading byte (discard it)
            return;
            // In the multi-byte sequence
        }
        else {
            // Broken sequence?
            if (((BYTE)c & 0xC0) != 0x80) {
                // Discard the sequense
                pb->ct = 0;
                continue;
            }
            // Save the trailing byte
            pb->bs[++pb->wi] = (BYTE)c;
            // End of the sequence?
            if (--pb->ct == 0) {
                break;
            }
            return;
        }
    }
    tp = (const TCHAR*)pb->bs;
    // UTF-8 ==> UTF-16
    dc = tchar2uni(&tp);
    // Wrong code?
    if (dc == 0xFFFFFFFF) {
        return;
    }
    hs = (WCHAR)(dc >> 16);
    wc = (WCHAR)dc;
    // UTF-32 input
#            elif FF_LFN_UNICODE == 3
    // Discard invalid code
    if (IsSurrogate(c) || c >= 0x110000) {
        return;
    }
    // Out of BMP?
    if (c >= 0x10000) {
        // Make high surrogate
        hs = (WCHAR)(0xD800 | ((c >> 10) - 0x40));
        // Make low surrogate
        wc = 0xDC00 | (c & 0x3FF);
    }
    else {
        hs = 0;
        wc = (WCHAR)c;
    }
#            endif
// A code point in UTF-16 is available in hs and wc

// Write a code point in UTF-16LE
#            if FF_STRF_ENCODE == 1
    // Surrogate pair?
    if (hs != 0) {
        st_word(&pb->buf[i], hs);
        i += 2;
        nc++;
    }
    st_word(&pb->buf[i], wc);
    i += 2;
    // Write a code point in UTF-16BE
#            elif FF_STRF_ENCODE == 2
    // Surrogate pair?
    if (hs != 0) {
        pb->buf[i++] = (BYTE)(hs >> 8);
        pb->buf[i++] = (BYTE)hs;
        nc++;
    }
    pb->buf[i++] = (BYTE)(wc >> 8);
    pb->buf[i++] = (BYTE)wc;
    // Write a code point in UTF-8
#            elif FF_STRF_ENCODE == 3
    // 4-byte sequence?
    if (hs != 0) {
        nc           += 3;
        hs            = (hs & 0x3FF) + 0x40;
        pb->buf[i++]  = (BYTE)(0xF0 | hs >> 8);
        pb->buf[i++]  = (BYTE)(0x80 | (hs >> 2 & 0x3F));
        pb->buf[i++]  = (BYTE)(0x80 | (hs & 3) << 4 | (wc >> 6 & 0x0F));
        pb->buf[i++]  = (BYTE)(0x80 | (wc & 0x3F));
    }
    else {
        // Single byte?
        if (wc < 0x80) {
            pb->buf[i++] = (BYTE)wc;
        }
        else {
            // 2-byte sequence?
            if (wc < 0x800) {
                nc           += 1;
                pb->buf[i++]  = (BYTE)(0xC0 | wc >> 6);
                // 3-byte sequence
            }
            else {
                nc           += 2;
                pb->buf[i++]  = (BYTE)(0xE0 | wc >> 12);
                pb->buf[i++]  = (BYTE)(0x80 | (wc >> 6 & 0x3F));
            }
            pb->buf[i++] = (BYTE)(0x80 | (wc & 0x3F));
        }
    }
    // Write a code point in ANSI/OEM
#            else
    if (hs != 0) {
        return;
    }
    // UTF-16 ==> ANSI/OEM
    wc = ff_uni2oem(wc, CODEPAGE);
    if (wc == 0) {
        return;
    }
    if (wc >= 0x100) {
        pb->buf[i++] = (BYTE)(wc >> 8);
        nc++;
    }
    pb->buf[i++] = (BYTE)wc;
#            endif

// ANSI/OEM input (without re-encoding)
#        else
    pb->buf[i++] = (BYTE)c;
#        endif

    // Write buffered characters to the file
    if (i >= (int)(sizeof pb->buf) - 4) {
        f_write(pb->fp, pb->buf, (UINT)i, &n);
        i = (n == (UINT)i) ? 0 : -1;
    }
    pb->idx  = i;
    pb->nchr = nc + 1;
}

// Flush remaining characters in the buffer

static int putc_flush(putbuff* pb) {
    UINT nw;

    // Flush buffered characters to the file
    if (pb->idx >= 0 && f_write(pb->fp, pb->buf, (UINT)pb->idx, &nw) == FR_OK
        && (UINT)pb->idx == nw) {
        return pb->nchr;
    }
    return -1;
}

// Initialize write buffer

static void putc_init(putbuff* pb, FIL* fp) {
    memset(pb, 0, sizeof(putbuff));
    pb->fp = fp;
}

int f_putc(TCHAR c, FIL* fp) {
    putbuff pb;

    putc_init(&pb, fp);
    // Put the character
    putc_bfd(&pb, c);
    return putc_flush(&pb);
}

//-----------------------------------------------------------------------

// Put a String to the File

//-----------------------------------------------------------------------

int f_puts(const TCHAR* str, FIL* fp) {
    putbuff pb;

    putc_init(&pb, fp);
    // Put the string
    while (*str) {
        putc_bfd(&pb, *str++);
    }
    return putc_flush(&pb);
}

//-----------------------------------------------------------------------

// Put a Formatted String to the File (with sub-functions)

//-----------------------------------------------------------------------

#        if FF_PRINT_FLOAT && FF_INTDEF == 2
#            include <math.h>

// Calculate log10(n) in integer output
static int ilog10(double n) {
    int rv = 0;

    // Decimate digit in right shift
    while (n >= 10) {
        if (n >= 100000) {
            n  /= 100000;
            rv += 5;
        }
        else {
            n /= 10;
            rv++;
        }
    }
    // Decimate digit in left shift
    while (n < 1) {
        if (n < 0.00001) {
            n  *= 100000;
            rv -= 5;
        }
        else {
            n *= 10;
            rv--;
        }
    }
    return rv;
}

// Calculate 10^n in integer input
static double i10x(int n) {
    double rv = 1;

    // Left shift
    while (n > 0) {
        if (n >= 5) {
            rv *= 100000;
            n  -= 5;
        }
        else {
            rv *= 10;
            n--;
        }
    }
    // Right shift
    while (n < 0) {
        if (n <= -5) {
            rv /= 100000;
            n  += 5;
        }
        else {
            rv /= 10;
            n++;
        }
    }
    return rv;
}

static void ftoa(
  // Buffer to output the floating point string
  char*  buf,
  // Value to output
  double val,
  // Number of fractional digits
  int    prec,
  // Notation
  TCHAR  fmt) {
    int         d;
    int         e = 0, m = 0;
    char        sign = 0;
    double      w;
    const char* er = 0;
    const char  ds = FF_PRINT_FLOAT == 2 ? ',' : '.';

    // Not a number?
    if (isnan(val)) {
        er = "NaN";
    }
    else {
        // Default precision? (6 fractional digits)
        if (prec < 0) {
            prec = 6;
        }
        // Negative?
        if (val < 0) {
            val  = 0 - val;
            sign = '-';
        }
        else {
            sign = '+';
        }
        // Infinite?
        if (isinf(val)) {
            er = "INF";
        }
        else {
            // Decimal notation?
            if (fmt == 'f') {
                // Round (nearest)
                val += i10x(0 - prec) / 2;
                m    = ilog10(val);
                if (m < 0) {
                    m = 0;
                }
                // Buffer overflow?
                if (m + prec + 3 >= SZ_NUM_BUF) {
                    er = "OV";
                }
                // E notation
            }
            else {
                // Not a true zero?
                if (val != 0) {
                    // Round (nearest)
                    val += i10x(ilog10(val) - prec) / 2;
                    e    = ilog10(val);
                    // Buffer overflow or E > +99?
                    if (e > 99 || prec + 7 >= SZ_NUM_BUF) {
                        er = "OV";
                    }
                    else {
                        if (e < -99) {
                            e = -99;
                        }
                        // Normalize
                        val /= i10x(e);
                    }
                }
            }
        }
        // Not error condition
        if (!er) {
            // Add a - if negative value
            if (sign == '-') {
                *buf++ = sign;
            }
            // Put decimal number
            do {
                // Insert a decimal separator when get into fractional part
                if (m == -1) {
                    *buf++ = ds;
                }
                // Snip the highest digit d
                w       = i10x(m);
                d       = (int)(val / w);
                val    -= d * w;
                // Put the digit
                *buf++  = (char)('0' + d);
                // Output all digits specified by prec
            } while (--m >= -prec);
            // Put exponent if needed
            if (fmt != 'f') {
                *buf++ = (char)fmt;
                if (e < 0) {
                    e      = 0 - e;
                    *buf++ = '-';
                }
                else {
                    *buf++ = '+';
                }
                *buf++ = (char)('0' + e / 10);
                *buf++ = (char)('0' + e % 10);
            }
        }
    }
    // Error condition
    if (er) {
        // Add sign if needed
        if (sign) {
            *buf++ = sign;
        }
        // Put error symbol
        do {
            *buf++ = *er++;
        } while (*er);
    }
    // Term
    *buf = 0;
}

// FF_PRINT_FLOAT && FF_INTDEF == 2
#        endif

int f_printf(FIL* fp, const TCHAR* fmt, ...) {
    va_list arp;
    putbuff pb;
    UINT    i, j, w, f, r;
    int     prec;
#        if FF_PRINT_LLI && FF_INTDEF == 2
    QWORD v;
#        else
    DWORD v;
#        endif
    TCHAR* tp;
    TCHAR  tc, pad;
    TCHAR  nul = 0;
    char   d, str[SZ_NUM_BUF];

    putc_init(&pb, fp);

    va_start(arp, fmt);

    for (;;) {
        tc = *fmt++;
        // End of format string
        if (tc == 0) {
            break;
        }
        // Not an escape character (pass-through)
        if (tc != '%') {
            putc_bfd(&pb, tc);
            continue;
        }
        // Initialize parms
        f = w = 0;
        pad   = ' ';
        prec  = -1;
        tc    = *fmt++;
        // Flag: '0' padded
        if (tc == '0') {
            pad = '0';
            tc  = *fmt++;
            // Flag: Left aligned
        }
        else if (tc == '-') {
            f  = 2;
            tc = *fmt++;
        }
        // Minimum width from an argument
        if (tc == '*') {
            w  = va_arg(arp, int);
            tc = *fmt++;
        }
        else {
            // Minimum width
            while (IsDigit(tc)) {
                w  = w * 10 + tc - '0';
                tc = *fmt++;
            }
        }
        // Precision
        if (tc == '.') {
            tc = *fmt++;
            // Precision from an argument
            if (tc == '*') {
                prec = va_arg(arp, int);
                tc   = *fmt++;
            }
            else {
                prec = 0;
                // Precision
                while (IsDigit(tc)) {
                    prec = prec * 10 + tc - '0';
                    tc   = *fmt++;
                }
            }
        }
        // Size: long int
        if (tc == 'l') {
            f  |= 4;
            tc  = *fmt++;
#        if FF_PRINT_LLI && FF_INTDEF == 2
            // Size: long long int
            if (tc == 'l') {
                f  |= 8;
                tc  = *fmt++;
            }
#        endif
        }
        // End of format string
        if (tc == 0) {
            break;
        }
        // Atgument type is...
        switch (tc) {
                // Unsigned binary
            case 'b':
                r = 2;
                break;

                // Unsigned octal
            case 'o':
                r = 8;
                break;

                // Signed decimal
            case 'd':
            // Unsigned decimal
            case 'u':
                r = 10;
                break;

                // Unsigned hexadecimal (lower case)
            case 'x':
            // Unsigned hexadecimal (upper case)
            case 'X':
                r = 16;
                break;

                // Character
            case 'c':
                putc_bfd(&pb, (TCHAR)va_arg(arp, int));
                continue;

                // String
            case 's':
                // Get a pointer argument
                tp = va_arg(arp, TCHAR*);
                // Null ptr generates a null string
                if (!tp) {
                    tp = &nul;
                }
                // j = tcslen(tp)
                for (j = 0; tp[j]; j++)
                    ;
                // Limited length of string body
                if (prec >= 0 && j > (UINT)prec) {
                    j = prec;
                }
                // Left pads
                for (; !(f & 2) && j < w; j++) {
                    putc_bfd(&pb, pad);
                }
                // Body
                while (*tp && prec--) {
                    putc_bfd(&pb, *tp++);
                }
                // Right pads
                while (j++ < w) {
                    putc_bfd(&pb, ' ');
                }
                continue;
#        if FF_PRINT_FLOAT && FF_INTDEF == 2
                // Floating point (decimal)
            case 'f':
            // Floating point (e)
            case 'e':
            // Floating point (E)
            case 'E':
                // Make a floating point string
                ftoa(str, va_arg(arp, double), prec, tc);
                // Left pads
                for (j = strlen(str); !(f & 2) && j < w; j++) {
                    putc_bfd(&pb, pad);
                }
                // Body
                for (i = 0; str[i]; putc_bfd(&pb, str[i++]))
                    ;
                // Right pads
                while (j++ < w) {
                    putc_bfd(&pb, ' ');
                }
                continue;
#        endif
                // Unknown type (pass-through)
            default:
                putc_bfd(&pb, tc);
                continue;
        }

            // Get an integer argument and put it in numeral

#        if FF_PRINT_LLI && FF_INTDEF == 2
        // long long argument?
        if (f & 8) {
            v = (QWORD)va_arg(arp, long long);
            // long argument?
        }
        else if (f & 4) {
            v = (tc == 'd') ? (QWORD)(long long)va_arg(arp, long)
                            : (QWORD)va_arg(arp, unsigned long);
            // int/short/char argument
        }
        else {
            v = (tc == 'd') ? (QWORD)(long long)va_arg(arp, int)
                            : (QWORD)va_arg(arp, unsigned int);
        }
        // Negative value?
        if (tc == 'd' && (v & 0x8000000000000000)) {
            v  = 0 - v;
            f |= 1;
        }
#        else
        // long argument?
        if (f & 4) {
            v = (DWORD)va_arg(arp, long);
            // int/short/char argument
        }
        else {
            v = (tc == 'd') ? (DWORD)(long)va_arg(arp, int)
                            : (DWORD)va_arg(arp, unsigned int);
        }
        // Negative value?
        if (tc == 'd' && (v & 0x80000000)) {
            v  = 0 - v;
            f |= 1;
        }
#        endif
        i = 0;
        // Make an integer number string
        do {
            d  = (char)(v % r);
            v /= r;
            if (d > 9) {
                d += (tc == 'x') ? 0x27 : 0x07;
            }
            str[i++] = d + '0';
        } while (v && i < SZ_NUM_BUF);
        // Sign
        if (f & 1) {
            str[i++] = '-';
        }
        // Write it

        // Left pads
        for (j = i; !(f & 2) && j < w; j++) {
            putc_bfd(&pb, pad);
        }
        // Body
        do {
            putc_bfd(&pb, (TCHAR)str[--i]);
        } while (i);
        // Right pads
        while (j++ < w) {
            putc_bfd(&pb, ' ');
        }
    }

    va_end(arp);

    return putc_flush(&pb);
}

// !FF_FS_READONLY
#    endif
// FF_USE_STRFUNC
#endif

#if FF_CODE_PAGE == 0
//-----------------------------------------------------------------------

// Set Active Codepage for the Path Name

//-----------------------------------------------------------------------

FRESULT f_setcp(WORD cp) {
    static const WORD validcp[22]
      = { 437, 720, 737, 771, 775, 850, 852, 855, 857, 860, 861,
          862, 863, 864, 865, 866, 869, 932, 936, 949, 950, 0 };
    static const BYTE* const tables[22]
      = { Ct437, Ct720, Ct737, Ct771, Ct775, Ct850, Ct852, Ct855,
          Ct857, Ct860, Ct861, Ct862, Ct863, Ct864, Ct865, Ct866,
          Ct869, Dc932, Dc936, Dc949, Dc950, 0 };
    UINT i;

    // Find the code page
    for (i = 0; validcp[i] != 0 && validcp[i] != cp; i++)
        ;
    // Not found?
    if (validcp[i] != cp) {
        return FR_INVALID_PARAMETER;
    }

    CodePage = cp;
    // DBCS
    if (cp >= 900) {
        ExCvt  = 0;
        DbcTbl = tables[i];
        // SBCS
    }
    else {
        ExCvt  = tables[i];
        DbcTbl = 0;
    }
    return FR_OK;
}
#endif /* FF_CODE_PAGE == 0 */

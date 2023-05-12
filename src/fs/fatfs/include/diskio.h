
/**
 * @file diskio.h
 * @brief Low level disk interface module include file
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

#ifndef SIMPLEKERNEL_DISKIO_H
#define SIMPLEKERNEL_DISKIO_H

#ifndef _DISKIO_DEFINED
#    define _DISKIO_DEFINED

#    ifdef __cplusplus
extern "C" {
#    endif

/// Status of Disk Functions
typedef BYTE DSTATUS;

/**
 * @brief Results of Disk Functions
 */
typedef enum {
    /// 0: Successful
    RES_OK = 0,
    /// 1: R/W Error
    RES_ERROR,
    /// 2: Write Protected
    RES_WRPRT,
    /// 3: Not Ready
    RES_NOTRDY,
    /// 4: Invalid Parameter
    RES_PARERR
} DRESULT;

/**
 * @brief Inidialize a Drive
 * @param  _pdrv            Physical drive number to identify the drive
 * @return DSTATUS          result
 */
DSTATUS disk_initialize(BYTE _pdrv);

/**
 * @brief Get Drive Status
 * @param  _pdrv            Physical drive number to identify the drive
 * @return DSTATUS          Drive Status
 */
DSTATUS disk_status(BYTE _pdrv);

/**
 * @brief Read Sector(s)
 * @param  _pdrv            Physical drive number to identify the drive
 * @param  _buff            Data buffer to store read data
 * @param  _sector          Start sector in LBA
 * @param  _count           Number of sectors to read
 * @return DRESULT          result
 */
DRESULT disk_read(BYTE _pdrv, BYTE* _buff, LBA_t _sector, UINT _count);

/**
 * @brief Write Sector(s)
 * @param  _pdrv            Physical drive number to identify the drive
 * @param  _buff            Data to be written
 * @param  _sector          Start sector in LBA
 * @param  _count           Number of sectors to write
 * @return DRESULT          result
 */
DRESULT disk_write(BYTE _pdrv, const BYTE* _buff, LBA_t _sector, UINT _count);

/**
 * @brief Miscellaneous Functions
 * @param  _pdrv            Physical drive number (0..)
 * @param  _cmd             Control code
 * @param  _buff            Buffer to send/receive control data
 * @return DRESULT          result
 */
DRESULT disk_ioctl(BYTE _pdrv, BYTE _cmd, void* _buff);

/// Disk Status Bits (DSTATUS)
#    define STA_OK           0x00
/// Drive not initialized
#    define STA_NOINIT       0x01
/// No medium in the drive
#    define STA_NODISK       0x02
/// Write protected
#    define STA_PROTECT      0x04

/// Command code for disk_ioctl function

/// Generic command (Used by FatFs)
/// Complete pending write process (needed at FF_FS_READONLY == 0)
#    define CTRL_SYNC        0
/// Get media size (needed at FF_USE_MKFS == 1)
#    define GET_SECTOR_COUNT 1
/// Get sector size (needed at FF_MAX_SS != FF_MIN_SS)
#    define GET_SECTOR_SIZE  2
/// Get erase block size (needed at FF_USE_MKFS == 1)
#    define GET_BLOCK_SIZE   3
/// Inform device that the data on the block of sectors is no longer used
/// (needed at FF_USE_TRIM == 1)
#    define CTRL_TRIM        4

/// Generic command (Not used by FatFs)
/// Get/Set power status
#    define CTRL_POWER       5
/// Lock/Unlock media removal
#    define CTRL_LOCK        6
/// Eject media
#    define CTRL_EJECT       7
/// Create physical format on the media
#    define CTRL_FORMAT      8

/// MMC/SDC specific ioctl command
/// Get card type
#    define MMC_GET_TYPE     10
/// Get CSD
#    define MMC_GET_CSD      11
/// Get CID
#    define MMC_GET_CID      12
/// Get OCR
#    define MMC_GET_OCR      13
/// Get SD status
#    define MMC_GET_SDSTAT   14
/// Read data form SD iSDIO register
#    define ISDIO_READ       55
/// Write data to SD iSDIO register
#    define ISDIO_WRITE      56
/// Masked write data to SD iSDIO register
#    define ISDIO_MRITE      57

/// ATA/CF specific ioctl command
/// Get F/W revision
#    define ATA_GET_REV      20
/// Get model name
#    define ATA_GET_MODEL    21
/// Get serial number
#    define ATA_GET_SN       22

#    ifdef __cplusplus
}
#    endif

#endif

#endif /* SIMPLEKERNEL_DISKIO_H */

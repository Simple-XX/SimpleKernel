
/**
 * @file diskio.c
 * @brief fat 文件系统底层 io 实现
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

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

// Obtains integer types
#include "ff.h"
// Declarations of disk functions
#include "diskio.h"

// Definitions of physical drive number for each drive
/// Example: Map Ramdisk to physical drive 0
#define DEV_RAM 0
/// Example: Map MMC/SD card to physical drive 1
#define DEV_MMC 1
/// Example: Map USB MSD to physical drive 2
#define DEV_USB 2

/**
 * @brief Get Drive Status
 * @param  _pdrv            Physical drive number to identify the drive
 * @return DSTATUS          Drive Status
 */
DSTATUS disk_status(BYTE _pdrv) {
    DSTATUS stat;
    int     result;

    switch (_pdrv) {
        case DEV_RAM:
            result = RAM_disk_status();

            // translate the result code here

            return stat;

        case DEV_MMC:
            result = MMC_disk_status();

            // translate the result code here

            return stat;

        case DEV_USB:
            result = USB_disk_status();

            // translate the result code here

            return stat;
    }
    return STA_NOINIT;
}

/**
 * @brief Inidialize a Drive
 * @param  _pdrv            Physical drive number to identify the drive
 * @return DSTATUS          result
 */
DSTATUS disk_initialize(BYTE _pdrv) {
    DSTATUS stat;
    int     result;

    switch (_pdrv) {
        case DEV_RAM:
            result = RAM_disk_initialize();

            // translate the result code here

            return stat;

        case DEV_MMC:
            result = MMC_disk_initialize();

            // translate the result code here

            return stat;

        case DEV_USB:
            result = USB_disk_initialize();

            // translate the result code here

            return stat;
    }
    return STA_NOINIT;
}

/**
 * @brief Read Sector(s)
 * @param  _pdrv            Physical drive number to identify the drive
 * @param  _buff            Data buffer to store read data
 * @param  _sector          Start sector in LBA
 * @param  _count           Number of sectors to read
 * @return DRESULT          result
 */
DRESULT disk_read(BYTE _pdrv, BYTE* _buff, LBA_t _sector, UINT _count) {
    DRESULT res;
    int     result;

    switch (_pdrv) {
        case DEV_RAM:
            // translate the arguments here

            result = RAM_disk_read(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_MMC:
            // translate the arguments here

            result = MMC_disk_read(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_USB:
            // translate the arguments here

            result = USB_disk_read(_buff, _sector, _count);

            // translate the result code here

            return res;
    }

    return RES_PARERR;
}

#if FF_FS_READONLY == 0

/**
 * @brief Write Sector(s)
 * @param  _pdrv            Physical drive number to identify the drive
 * @param  _buff            Data to be written
 * @param  _sector          Start sector in LBA
 * @param  _count           Number of sectors to write
 * @return DRESULT          result
 */
DRESULT disk_write(BYTE _pdrv, const BYTE* _buff, LBA_t _sector, UINT _count) {
    DRESULT res;
    int     result;

    switch (_pdrv) {
        case DEV_RAM:
            // translate the arguments here

            result = RAM_disk_write(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_MMC:
            // translate the arguments here

            result = MMC_disk_write(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_USB:
            // translate the arguments here

            result = USB_disk_write(_buff, _sector, _count);

            // translate the result code here

            return res;
    }

    return RES_PARERR;
}

#endif

/**
 * @brief Miscellaneous Functions
 * @param  _pdrv            Physical drive number (0..)
 * @param  _cmd             Control code
 * @param  _buff            Buffer to send/receive control data
 * @return DRESULT          result
 */
DRESULT disk_ioctl(BYTE _pdrv, BYTE _cmd, void* _buff) {
    DRESULT res;
    int     result;

    switch (_pdrv) {
        case DEV_RAM:

            // Process of the command for the RAM drive

            return res;

        case DEV_MMC:

            // Process of the command for the MMC/SD card

            return res;

        case DEV_USB:

            // Process of the command the USB drive

            return res;
    }

    return RES_PARERR;
}

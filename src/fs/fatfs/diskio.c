
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

/* Obtains integer types */
#include "ff.h"
/* Declarations of disk functions */
#include "diskio.h"

/* Definitions of physical drive number for each drive */
/* Example: Map Ramdisk to physical drive 0 */
#define DEV_RAM 0
/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_MMC 1
/* Example: Map USB MSD to physical drive 2 */
#define DEV_USB 2

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
  /* Physical drive number to identify the drive */
  BYTE _pdrv) {
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

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
  /* Physical drive number to identify the drive */
  BYTE _pdrv) {
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

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
  /* Physical drive number to identify the drive */
  BYTE  _pdrv,
  /* Data buffer to store read data */
  BYTE* _buff,
  /* Start sector in LBA */
  LBA_t _sector,
  /* Number of sectors to read */
  UINT  _count) {
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

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
  /* Physical drive number to identify the drive */
  BYTE        _pdrv,
  /* Data to be written */
  const BYTE* _buff,
  /* Start sector in LBA */
  LBA_t       _sector,
  /* Number of sectors to write */
  UINT        _count) {
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

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
  /* Physical drive number (0..) */
  BYTE  _pdrv,
  /* Control code */
  BYTE  _cmd,
  /* Buffer to send/receive control data */
  void* _buff) {
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

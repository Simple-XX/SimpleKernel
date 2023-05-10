
/**
 * @file diskio.c
 * @brief Low level disk I/O module SKELETON for FatFs
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

#include "buf.h"
#include "dev_drv_manager.h"
#include "device_base.h"

// Definitions of physical drive number for each drive
/// Example: Map Ramdisk to physical drive 0
#define DEV_RAM 0
/// Example: Map MMC/SD card to physical drive 1
#define DEV_MMC 1
/// Example: Map USB MSD to physical drive 2
#define DEV_USB 2

DWORD get_fattime(void) {
    return 0;
}

DSTATUS disk_status(BYTE _pdrv) {
    DSTATUS stat;
    int     result;

    switch (_pdrv) {
        case DEV_RAM:
            // result = RAM_disk_status();

            // translate the result code here

            return stat;

        case DEV_MMC:
            // result = MMC_disk_status();

            // translate the result code here

            return stat;

        case DEV_USB:
            // result = USB_disk_status();

            // translate the result code here

            return stat;
    }
    return STA_NOINIT;
}

extern void virtio_disk_init();

DSTATUS     disk_initialize(BYTE _pdrv) {
    DSTATUS stat;
    int     result;
    switch (_pdrv) {
        case DEV_RAM:
            // result = RAM_disk_initialize();

            // translate the result code here

            return stat;

        case DEV_MMC:
            // result = MMC_disk_initialize();

            // translate the result code here

            return stat;

        case DEV_USB:
            // result = USB_disk_initialize();

            // translate the result code here

            return stat;
    }
    return STA_NOINIT;
}

DRESULT disk_read(BYTE _pdrv, BYTE* _buff, LBA_t _sector, UINT _count) {
    DRESULT res;
    int     result;

    info("disk_read [%d] _sector: 0x%X, _count: 0x%X\n", _pdrv, _sector,
         _count);

    device_base_t* dev
      = (device_base_t*)DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(1);

    buf_t buf;
    buf.sector = _sector;

    dev->read(buf);

    memcpy(_buff, buf.data, COMMON::BUFFFER_SIZE);

// #define DEBUG
#ifdef DEBUG
    for (size_t i = 0; i < COMMON::BUFFFER_SIZE; i++) {
        printf("0x%X ", buf.data[i]);
    }

#    undef DEBUG
#endif
    switch (_pdrv) {
        case DEV_RAM:
            // translate the arguments here

            // result = RAM_disk_read(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_MMC:
            // translate the arguments here

            // result = MMC_disk_read(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_USB:
            // translate the arguments here

            // result = USB_disk_read(_buff, _sector, _count);

            // translate the result code here

            return res;
    }

    return RES_PARERR;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE _pdrv, const BYTE* _buff, LBA_t _sector, UINT _count) {
    DRESULT res;
    int     result;
    info("disk_write [%d] _sector: 0x%X, _count: 0x%X\n", _pdrv, _sector,
         _count);

    device_base_t* dev
      = (device_base_t*)DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(1);

    buf_t buf;
    buf.sector = _sector;
    memcpy(buf.data, _buff, COMMON::BUFFFER_SIZE);

    dev->write(buf);

    warn("write done %d\n", 0);

    switch (_pdrv) {
        case DEV_RAM:
            // translate the arguments here

            // result = RAM_disk_write(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_MMC:
            // translate the arguments here

            // result = MMC_disk_write(_buff, _sector, _count);

            // translate the result code here

            return res;

        case DEV_USB:
            // translate the arguments here

            // result = USB_disk_write(_buff, _sector, _count);

            // translate the result code here

            return res;
    }

    return RES_PARERR;
}

#endif

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

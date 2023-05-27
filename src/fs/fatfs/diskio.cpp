
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
#define DEV_MMIO 0
/// Example: Map MMC/SD card to physical drive 1
#define DEV_MMC  1
/// Example: Map USB MSD to physical drive 2
#define DEV_USB  2

DWORD get_fattime(void) {
    return 0;
}

DSTATUS disk_status(BYTE _pdrv) {
    DSTATUS status = STA_OK;
    switch (_pdrv) {
        case DEV_MMIO: {
            break;
        }
        case DEV_MMC: {
            break;
        }
        case DEV_USB: {
            break;
        }
        default: {
            warn("disk_status unhandled device.\n");
            break;
        }
    }
    return status;
}

DSTATUS disk_initialize(BYTE _pdrv) {
    DSTATUS status = STA_OK;
    switch (_pdrv) {
        case DEV_MMIO: {
            break;
        }
        case DEV_MMC: {
            break;
        }
        case DEV_USB: {
            break;
        }
        default: {
            warn("disk_initialize unhandled device.\n");
            break;
        }
    }
    return status;
}

DRESULT disk_read(BYTE _pdrv, BYTE* _buff, LBA_t _sector, UINT _count) {
    assert(_count == 1);
    DRESULT result = RES_OK;
    info("disk_read [%d] _sector: 0x%X, _count: 0x%X\n", _pdrv, _sector,
         _count);

    switch (_pdrv) {
        case DEV_MMIO: {
            device_base_t* dev = (device_base_t*)DEV_DRV_MANAGER::get_instance()
                                   .get_dev_via_intr_no(1);

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
            break;
        }
        case DEV_MMC: {
            break;
        }
        case DEV_USB: {
            break;
        }
        default: {
            warn("disk_read unhandled device.\n");
            break;
        }
    }

    warn("read done\n");
    return result;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE _pdrv, const BYTE* _buff, LBA_t _sector, UINT _count) {
    assert(_count == 1);
    DRESULT result = RES_OK;
    info("disk_write [%d] _sector: 0x%X, _count: 0x%X\n", _pdrv, _sector,
         _count);

    switch (_pdrv) {
        case DEV_MMIO: {
            device_base_t* dev = (device_base_t*)DEV_DRV_MANAGER::get_instance()
                                   .get_dev_via_intr_no(1);
            buf_t buf;
            buf.sector = _sector;

            memcpy(buf.data, _buff, COMMON::BUFFFER_SIZE);

            dev->write(buf);

            break;
        }
        case DEV_MMC: {
            break;
        }
        case DEV_USB: {
            break;
        }
        default: {
            warn("disk_write unhandled device.\n");
            break;
        }
    }

    warn("write done\n");
    return result;
}

#endif /* FF_FS_READONLY == 0 */

DRESULT disk_ioctl(BYTE _pdrv, BYTE _cmd, void* _buff) {
    (void)_cmd;
    (void)_buff;
    DRESULT result = RES_OK;
    switch (_pdrv) {
        case DEV_MMIO: {
            break;
        }
        case DEV_MMC: {
            break;
        }
        case DEV_USB: {
            break;
        }
        default: {
            warn("disk_ioctl unhandled device.\n");
            break;
        }
    }

    return result;
}

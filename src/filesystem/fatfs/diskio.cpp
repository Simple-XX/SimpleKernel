/**
 * @copyright Copyright The SimpleKernel Contributors
 */

// ff.h 必须在 diskio.h 之前包含
// clang-format off
#include <ff.h>
#include <diskio.h>
// clang-format on

#include "fatfs.hpp"
#include "kernel_log.hpp"

extern "C" {

/**
 * @brief 返回 FAT 时间戳。无 RTC 时返回 0（epoch）。
 *
 * @note FF_FS_READONLY == 0 时 FatFS 要求此函数存在。
 * @return DWORD 始终返回 0。
 */
auto get_fattime() -> DWORD { return 0; }

/**
 * @brief 查询磁盘驱动器状态。
 *
 * @param pdrv 物理驱动器编号。
 * @return DSTATUS 驱动器状态标志；如果设备未注册则返回 STA_NOINIT。
 */
auto disk_status(BYTE pdrv) -> DSTATUS {
  if (fatfs::FatFsFileSystem::GetBlockDevice(pdrv) == nullptr) {
    return STA_NOINIT;
  }
  return 0;
}

/**
 * @brief 初始化磁盘驱动器。
 *
 * @note BlockDevice 已由调用方在注册时完成初始化，此函数仅检查设备是否存在。
 * @param pdrv 物理驱动器编号。
 * @return DSTATUS 驱动器状态标志；如果设备未注册则返回 STA_NOINIT。
 */
auto disk_initialize(BYTE pdrv) -> DSTATUS {
  if (fatfs::FatFsFileSystem::GetBlockDevice(pdrv) == nullptr) {
    return STA_NOINIT;
  }
  return 0;
}

/**
 * @brief 从磁盘读取扇区。
 *
 * @param pdrv   物理驱动器编号。
 * @param buff   目标缓冲区指针。
 * @param sector 起始逻辑块地址。
 * @param count  要读取的扇区数。
 * @return DRESULT 操作结果；RES_OK 表示成功。
 */
auto disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) -> DRESULT {
  auto* dev = fatfs::FatFsFileSystem::GetBlockDevice(pdrv);
  if (dev == nullptr) {
    return RES_NOTRDY;
  }
  auto result = dev->ReadSectors(sector, static_cast<uint32_t>(count), buff);
  if (!result) {
    klog::Err("disk_read: pdrv={} sector={} count={} failed",
              static_cast<unsigned>(pdrv), static_cast<uint64_t>(sector),
              static_cast<unsigned>(count));
    return RES_ERROR;
  }
  return RES_OK;
}

/**
 * @brief 向磁盘写入扇区。
 *
 * @param pdrv   物理驱动器编号。
 * @param buff   源数据缓冲区指针。
 * @param sector 起始逻辑块地址。
 * @param count  要写入的扇区数。
 * @return DRESULT 操作结果；RES_OK 表示成功。
 */
auto disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
    -> DRESULT {
  auto* dev = fatfs::FatFsFileSystem::GetBlockDevice(pdrv);
  if (dev == nullptr) {
    return RES_NOTRDY;
  }
  auto result = dev->WriteSectors(sector, static_cast<uint32_t>(count), buff);
  if (!result) {
    klog::Err("disk_write: pdrv={} sector={} count={} failed",
              static_cast<unsigned>(pdrv), static_cast<uint64_t>(sector),
              static_cast<unsigned>(count));
    return RES_ERROR;
  }
  return RES_OK;
}

/**
 * @brief 执行磁盘 I/O 控制操作。
 *
 * @param pdrv 物理驱动器编号。
 * @param cmd  控制命令（CTRL_SYNC、GET_SECTOR_COUNT、GET_SECTOR_SIZE 等）。
 * @param buff 命令参数/结果缓冲区，含义取决于 cmd。
 * @return DRESULT 操作结果；不支持的命令返回 RES_PARERR。
 */
auto disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) -> DRESULT {
  auto* dev = fatfs::FatFsFileSystem::GetBlockDevice(pdrv);
  if (dev == nullptr) {
    return RES_NOTRDY;
  }
  switch (cmd) {
    case CTRL_SYNC: {
      auto r = dev->Flush();
      return r ? RES_OK : RES_ERROR;
    }
    case GET_SECTOR_COUNT:
      if (buff == nullptr) {
        return RES_PARERR;
      }
      static_cast<LBA_t*>(buff)[0] = static_cast<LBA_t>(dev->GetSectorCount());
      return RES_OK;
    case GET_SECTOR_SIZE:
      if (buff == nullptr) {
        return RES_PARERR;
      }
      static_cast<WORD*>(buff)[0] = static_cast<WORD>(dev->GetSectorSize());
      return RES_OK;
    case GET_BLOCK_SIZE:
      if (buff == nullptr) {
        return RES_PARERR;
      }
      static_cast<DWORD*>(buff)[0] = 1;
      return RES_OK;
    case CTRL_TRIM:
      return RES_OK;  // TRIM 不是必需的
    default:
      return RES_PARERR;
  }
}
}

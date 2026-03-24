/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "device_manager.hpp"
#include "device_node.hpp"
#include "fatfs.hpp"
#include "kernel_log.hpp"
#include "mount.hpp"
#include "ramfs.hpp"
#include "vfs.hpp"

/// @brief FatFS 逻辑驱动器号（对应 rootfs.img）
static constexpr uint8_t kRootFsDriveId = 0;

/// @brief 文件系统子系统初始化入口
auto FileSystemInit() -> void {
  // 初始化 VFS
  auto init_result = vfs::Init();
  if (!init_result.has_value()) {
    klog::Err("FileSystemInit: vfs::Init failed: {}",
              init_result.error().message());
    return;
  }

  // 创建 ramfs 根文件系统并挂载到 "/"
  static ramfs::RamFs root_ramfs;
  auto mount_result = vfs::GetMountTable().Mount("/", &root_ramfs, nullptr);
  if (!mount_result.has_value()) {
    klog::Err("FileSystemInit: failed to mount ramfs at /: {}",
              mount_result.error().message());
    return;
  }

  // Mount FatFS on the first available block device at /mnt/fat
  DeviceNode* blk_nodes[4]{};
  const size_t blk_count = DeviceManagerSingleton::instance().FindDevicesByType(
      DeviceType::kBlock, blk_nodes, 4);

  if (blk_count > 0 && blk_nodes[0]->block_device != nullptr) {
    auto* blk = blk_nodes[0]->block_device;
    static fatfs::FatFsFileSystem fat_fs(kRootFsDriveId);
    auto fat_mount = fat_fs.Mount(blk);
    if (!fat_mount.has_value()) {
      klog::Err("FileSystemInit: FatFsFileSystem::Mount failed: {}",
                fat_mount.error().message());
    } else {
      auto vfs_mount = vfs::GetMountTable().Mount("/mnt/fat", &fat_fs, blk);
      if (!vfs_mount.has_value()) {
        klog::Err("FileSystemInit: vfs mount at /mnt/fat failed: {}",
                  vfs_mount.error().message());
      } else {
        klog::Info("FileSystemInit: FatFS mounted at /mnt/fat (device: {})",
                   blk->GetName());
      }
    }
  }

  klog::Info("FileSystemInit: complete");
}

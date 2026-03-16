/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cstddef>
#include <cstdint>

#include "device_manager.hpp"
#include "device_node.hpp"
#include "fatfs.hpp"
#include "kstd_cstring"
#include "mount.hpp"
#include "system_test.h"
#include "vfs.hpp"

auto fatfs_system_test() -> bool {
  klog::Info("fatfs_system_test: start");

  // T1: Get virtio-blk device via DeviceManager
  DeviceNode* blk_nodes[4]{};
  const size_t blk_count = DeviceManagerSingleton::instance().FindDevicesByType(
      DeviceType::kBlock, blk_nodes, 4);
  if (blk_count == 0 || blk_nodes[0]->block_device == nullptr) {
    klog::Info(
        "fatfs_system_test: SKIP — no virtio-blk device "
        "available");
    return true;  // Graceful skip, not a failure
  }
  vfs::BlockDevice* blk = blk_nodes[0]->block_device;
  klog::Info("fatfs_system_test: virtio-blk device: {}", blk->GetName());
  EXPECT_GT(blk->GetSectorCount(), static_cast<uint64_t>(0),
            "fatfs_system_test: virtio-blk has zero sectors");

  // T2: Mount FatFS at /mnt/fat
  // vfs::Init() and ramfs mount at "/" should already have been done by
  // ramfs_system_test, but call Init() again (it's idempotent).
  auto init_result = vfs::Init();
  EXPECT_TRUE(init_result.has_value(), "fatfs_system_test: vfs init failed");

  // Create /mnt and /mnt/fat directories in the VFS tree (in ramfs at /)
  // before mounting. MkDir is idempotent-ish — ignore errors if exists.
  (void)vfs::MkDir("/mnt");
  (void)vfs::MkDir("/mnt/fat");

  static fatfs::FatFsFileSystem fat_fs(0);
  auto fat_mount = fat_fs.Mount(blk);
  EXPECT_TRUE(fat_mount.has_value(),
              "fatfs_system_test: FatFsFileSystem::Mount failed");
  klog::Info("fatfs_system_test: FatFsFileSystem::Mount ok");

  auto vfs_mount = vfs::GetMountTable().Mount("/mnt/fat", &fat_fs, blk);
  EXPECT_TRUE(vfs_mount.has_value(),
              "fatfs_system_test: vfs mount at /mnt/fat failed");
  klog::Info("fatfs_system_test: vfs mount at /mnt/fat ok");

  // T3: Write a file on the FAT volume
  {
    auto file_result =
        vfs::Open("/mnt/fat/test.txt",
                  vfs::OpenFlags::kOCreate | vfs::OpenFlags::kOReadWrite);
    EXPECT_TRUE(file_result.has_value(),
                "fatfs_system_test: open /mnt/fat/test.txt failed");
    vfs::File* file = file_result.value();

    const char kMsg[] = "Hello, FatFS!";
    auto write_result = vfs::Write(file, kMsg, sizeof(kMsg) - 1);
    EXPECT_TRUE(write_result.has_value(),
                "fatfs_system_test: write to /mnt/fat/test.txt failed");
    EXPECT_EQ(write_result.value(), sizeof(kMsg) - 1,
              "fatfs_system_test: write byte count mismatch");
    klog::Info("fatfs_system_test: wrote {} bytes to /mnt/fat/test.txt",
               write_result.value());

    (void)vfs::Close(file);
  }

  // T4: Read back and verify
  {
    auto file_result =
        vfs::Open("/mnt/fat/test.txt", vfs::OpenFlags::kOReadOnly);
    EXPECT_TRUE(file_result.has_value(),
                "fatfs_system_test: re-open /mnt/fat/test.txt failed");
    vfs::File* file = file_result.value();

    char buf[64] = {};
    const char kMsg[] = "Hello, FatFS!";
    auto read_result = vfs::Read(file, buf, sizeof(buf) - 1);
    EXPECT_TRUE(read_result.has_value(),
                "fatfs_system_test: read from /mnt/fat/test.txt failed");
    EXPECT_EQ(read_result.value(), sizeof(kMsg) - 1,
              "fatfs_system_test: read byte count mismatch");
    EXPECT_EQ(memcmp(buf, kMsg, sizeof(kMsg) - 1), 0,
              "fatfs_system_test: read content mismatch");
    klog::Info("fatfs_system_test: verified read: {}", buf);

    (void)vfs::Close(file);
  }

  // T5: MkDir on FAT volume
  {
    auto mkdir_result = vfs::MkDir("/mnt/fat/subdir");
    EXPECT_TRUE(mkdir_result.has_value(),
                "fatfs_system_test: mkdir /mnt/fat/subdir failed");
    klog::Info("fatfs_system_test: mkdir /mnt/fat/subdir ok");

    // Create a file inside subdir
    auto inner =
        vfs::Open("/mnt/fat/subdir/inner.txt",
                  vfs::OpenFlags::kOCreate | vfs::OpenFlags::kOWriteOnly);
    EXPECT_TRUE(inner.has_value(),
                "fatfs_system_test: create /mnt/fat/subdir/inner.txt failed");
    if (inner.has_value()) {
      (void)vfs::Close(inner.value());
    }

    // ReadDir on root of fat volume to find subdir
    auto dir_file = vfs::Open(
        "/mnt/fat", vfs::OpenFlags::kOReadOnly | vfs::OpenFlags::kODirectory);
    EXPECT_TRUE(dir_file.has_value(),
                "fatfs_system_test: open /mnt/fat as dir failed");
    if (dir_file.has_value()) {
      vfs::DirEntry entries[16] = {};
      auto readdir_result = vfs::ReadDir(dir_file.value(), entries, 16);
      EXPECT_TRUE(readdir_result.has_value(),
                  "fatfs_system_test: readdir /mnt/fat failed");
      // Should see at least test.txt and subdir
      EXPECT_GT(readdir_result.value(), static_cast<size_t>(1),
                "fatfs_system_test: readdir /mnt/fat should return > 1 entry");
      klog::Info("fatfs_system_test: readdir /mnt/fat returned {} entries",
                 readdir_result.value());
      (void)vfs::Close(dir_file.value());
    }
  }

  // T6: Unmount and remount — verify persistence
  {
    auto unmount_result = fat_fs.Unmount();
    EXPECT_TRUE(unmount_result.has_value(),
                "fatfs_system_test: FatFsFileSystem::Unmount failed");
    klog::Info("fatfs_system_test: unmounted ok");

    // Remount
    auto remount_result = fat_fs.Mount(blk);
    EXPECT_TRUE(remount_result.has_value(),
                "fatfs_system_test: remount failed");
    klog::Info("fatfs_system_test: remounted ok");

    // Re-wire VFS mount
    auto vfs_remount = vfs::GetMountTable().Mount("/mnt/fat", &fat_fs, blk);
    EXPECT_TRUE(vfs_remount.has_value(),
                "fatfs_system_test: vfs remount failed");

    // Verify test.txt persisted
    auto file_result =
        vfs::Open("/mnt/fat/test.txt", vfs::OpenFlags::kOReadOnly);
    EXPECT_TRUE(file_result.has_value(),
                "fatfs_system_test: test.txt not found after remount");
    if (file_result.has_value()) {
      char buf[64] = {};
      const char kMsg[] = "Hello, FatFS!";
      auto read_result = vfs::Read(file_result.value(), buf, sizeof(buf) - 1);
      EXPECT_TRUE(read_result.has_value(),
                  "fatfs_system_test: read after remount failed");
      EXPECT_EQ(memcmp(buf, kMsg, sizeof(kMsg) - 1), 0,
                "fatfs_system_test: data corrupted after remount");
      klog::Info("fatfs_system_test: persistence verified: {}", buf);
      (void)vfs::Close(file_result.value());
    }
  }

  klog::Info("fatfs_system_test: all tests passed");
  return true;
}

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "fatfs.hpp"

#include <ff.h>

#include "kernel_log.hpp"
#include "kstd_cstring"
#include "vfs.hpp"

namespace fatfs {

std::array<vfs::BlockDevice*, FF_VOLUMES> FatFsFileSystem::block_devices_{};

namespace {

/**
 * @brief 将 FatFS FRESULT 映射到内核 ErrorCode
 * @details 区分常见错误类别，避免将所有错误笼统归入 kFsCorrupted
 */
auto FresultToErrorCode(FRESULT fr) -> ErrorCode {
  switch (fr) {
    case FR_OK:
      return ErrorCode::kSuccess;
    case FR_NO_FILE:
    case FR_NO_PATH:
      return ErrorCode::kFsFileNotFound;
    case FR_EXIST:
      return ErrorCode::kFsFileExists;
    case FR_DENIED:
    case FR_WRITE_PROTECTED:
      return ErrorCode::kFsPermissionDenied;
    case FR_NOT_READY:
    case FR_DISK_ERR:
      return ErrorCode::kBlkReadFailed;
    case FR_NOT_ENABLED:
    case FR_INVALID_DRIVE:
      return ErrorCode::kFsNotMounted;
    case FR_NOT_ENOUGH_CORE:
      return ErrorCode::kFsNoSpace;
    case FR_INVALID_NAME:
    case FR_INVALID_PARAMETER:
    case FR_INVALID_OBJECT:
      return ErrorCode::kInvalidArgument;
    case FR_TOO_MANY_OPEN_FILES:
      return ErrorCode::kFsFdTableFull;
    case FR_INT_ERR:
    case FR_MKFS_ABORTED:
    default:
      return ErrorCode::kFsCorrupted;
  }
}

/**
 * @brief 将 FRESULT 转换为 Expected<void>
 */
auto FresultToExpected(FRESULT fr) -> Expected<void> {
  if (fr == FR_OK) {
    return {};
  }
  return std::unexpected(Error{FresultToErrorCode(fr)});
}

/**
 * @brief 将 FILINFO 属性转换为 vfs::FileType
 */
auto FilInfoToFileType(const FILINFO& fi) -> vfs::FileType {
  if ((fi.fattrib & AM_DIR) != 0) {
    return vfs::FileType::kDirectory;
  }
  return vfs::FileType::kRegular;
}

}  // namespace

auto FatFsFileSystem::SetBlockDevice(uint8_t pdrv, vfs::BlockDevice* device)
    -> void {
  if (pdrv < FF_VOLUMES) {
    block_devices_[pdrv] = device;
  }
}

auto FatFsFileSystem::GetBlockDevice(uint8_t pdrv) -> vfs::BlockDevice* {
  if (pdrv >= FF_VOLUMES) {
    return nullptr;
  }
  return block_devices_[pdrv];
}

FatFsFileSystem::FatFsFileSystem(uint8_t volume_id)
    : volume_id_(volume_id),
      fatfs_obj_{},
      inodes_{},
      fil_pool_{},
      dir_pool_{},
      inode_ops_(this),
      file_ops_(this) {}

FatFsFileSystem::~FatFsFileSystem() {
  if (mounted_) {
    (void)Unmount();
  }
}

auto FatFsFileSystem::GetName() const -> const char* { return "fatfs"; }

auto FatFsFileSystem::Mount(vfs::BlockDevice* device) -> Expected<vfs::Inode*> {
  if (device == nullptr) {
    klog::Err("FatFsFileSystem::Mount: device is nullptr");
    return std::unexpected(Error{ErrorCode::kInvalidArgument});
  }
  if (volume_id_ >= FF_VOLUMES) {
    return std::unexpected(Error{ErrorCode::kInvalidArgument});
  }

  // 注册块设备，使 diskio.cpp 的回调能够访问它
  SetBlockDevice(volume_id_, device);

  // 构造 FatFS 挂载路径："0:/" 或 "1:/" 等
  char path[4] = {static_cast<char>('0' + volume_id_), ':', '/', '\0'};

  FRESULT fr = f_mount(&fatfs_obj_, path, 1);
  if (fr != FR_OK) {
    SetBlockDevice(volume_id_, nullptr);
    klog::Err("FatFsFileSystem::Mount: f_mount failed ({})",
              static_cast<int>(fr));
    return std::unexpected(Error{FresultToErrorCode(fr)});
  }

  // 构建根 inode
  FatInode* fi = AllocateFatInode();
  if (fi == nullptr) {
    (void)f_mount(nullptr, path, 0);
    SetBlockDevice(volume_id_, nullptr);
    return std::unexpected(Error{ErrorCode::kOutOfMemory});
  }
  fi->inode.ino = 0;
  fi->inode.type = vfs::FileType::kDirectory;
  fi->inode.size = 0;
  fi->inode.permissions = kRootDirPermissions;
  fi->inode.link_count = 1;
  fi->inode.fs = this;
  fi->inode.ops = &inode_ops_;
  fi->inode.fs_private = fi;
  strncpy(fi->path.data(), path, fi->path.size() - 1);
  fi->path[fi->path.size() - 1] = '\0';

  root_inode_ = &fi->inode;
  mounted_ = true;
  return root_inode_;
}

auto FatFsFileSystem::Unmount() -> Expected<void> {
  if (!mounted_) {
    return {};
  }
  char path[4] = {static_cast<char>('0' + volume_id_), ':', '/', '\0'};
  FRESULT fr = f_mount(nullptr, path, 0);
  SetBlockDevice(volume_id_, nullptr);
  mounted_ = false;
  root_inode_ = nullptr;
  for (auto& node : inodes_) {
    if (node.in_use) {
      if (node.dir != nullptr) {
        (void)f_closedir(node.dir);
        FreeDir(node.dir);
        node.dir = nullptr;
      }
    }
    node.in_use = false;
  }
  return FresultToExpected(fr);
}

auto FatFsFileSystem::Sync() -> Expected<void> {
  auto* dev = GetBlockDevice(volume_id_);
  if (dev != nullptr) {
    return dev->Flush();
  }
  return {};
}

auto FatFsFileSystem::AllocateInode() -> Expected<vfs::Inode*> {
  FatInode* fi = AllocateFatInode();
  if (fi == nullptr) {
    return std::unexpected(Error{ErrorCode::kOutOfMemory});
  }
  fi->inode.fs = this;
  fi->inode.ops = &inode_ops_;
  fi->inode.fs_private = fi;
  return &fi->inode;
}

auto FatFsFileSystem::FreeInode(vfs::Inode* inode) -> Expected<void> {
  if (inode == nullptr) {
    return std::unexpected(Error{ErrorCode::kInvalidArgument});
  }
  auto* fi = static_cast<FatInode*>(inode->fs_private);
  FreeFatInode(fi);
  return {};
}

auto FatFsFileSystem::GetFileOps() -> vfs::FileOps* { return &file_ops_; }

auto FatFsFileSystem::AllocateFatInode() -> FatInode* {
  for (auto& fi : inodes_) {
    if (!fi.in_use) {
      fi = FatInode{};
      fi.in_use = true;
      return &fi;
    }
  }
  return nullptr;
}

auto FatFsFileSystem::FreeFatInode(FatInode* fi) -> void {
  if (fi != nullptr) {
    fi->in_use = false;
  }
}

auto FatFsFileSystem::AllocateFil() -> FIL* {
  for (auto& fh : fil_pool_) {
    if (!fh.in_use) {
      fh = FatFileHandle{};
      fh.in_use = true;
      return &fh.fil;
    }
  }
  return nullptr;
}

auto FatFsFileSystem::FreeFil(FIL* fil) -> void {
  for (auto& fh : fil_pool_) {
    if (&fh.fil == fil) {
      fh.in_use = false;
      return;
    }
  }
}

auto FatFsFileSystem::AllocateDir() -> DIR* {
  for (auto& dh : dir_pool_) {
    if (!dh.in_use) {
      dh = FatDirHandle{};
      dh.in_use = true;
      return &dh.dir;
    }
  }
  return nullptr;
}

auto FatFsFileSystem::FreeDir(DIR* dir) -> void {
  for (auto& dh : dir_pool_) {
    if (&dh.dir == dir) {
      dh.in_use = false;
      return;
    }
  }
}

auto FatFsFileSystem::OpenFil(vfs::Inode* inode, vfs::OpenFlags open_flags)
    -> Expected<void> {
  auto* fi = static_cast<FatInode*>(inode->fs_private);
  if (fi->fil != nullptr) {
    // 已打开
    return {};
  }
  FIL* fil = AllocateFil();
  if (fil == nullptr) {
    return std::unexpected(Error{ErrorCode::kFsFdTableFull});
  }
  BYTE fa_mode = 0;
  // kOReadOnly == 0，需单独检查
  if (open_flags == vfs::OpenFlags::kOReadOnly) {
    fa_mode = FA_READ;
  }
  if ((open_flags & vfs::OpenFlags::kOWriteOnly) != 0U) {
    fa_mode = FA_WRITE;
  }
  if ((open_flags & vfs::OpenFlags::kOReadWrite) != 0U) {
    fa_mode = FA_READ | FA_WRITE;
  }
  if ((open_flags & vfs::OpenFlags::kOCreate) != 0U) {
    fa_mode |= FA_OPEN_ALWAYS;
  }
  if ((open_flags & vfs::OpenFlags::kOTruncate) != 0U) {
    fa_mode |= FA_CREATE_ALWAYS;
  }
  FRESULT fr = f_open(fil, fi->path.data(), fa_mode);
  if (fr != FR_OK) {
    FreeFil(fil);
    return std::unexpected(Error{FresultToErrorCode(fr)});
  }
  fi->fil = fil;
  return {};
}

auto FatFsFileSystem::FatFsInodeOps::Lookup(vfs::Inode* dir, const char* name)
    -> Expected<vfs::Inode*> {
  auto* dir_fi = static_cast<FatInode*>(dir->fs_private);

  // 拼接完整路径：dir_path + name
  std::array<char, kPathBufSize> full_path{};
  strncpy(full_path.data(), dir_fi->path.data(), full_path.size() - 1);
  size_t dir_len = strlen(full_path.data());
  strncpy(full_path.data() + dir_len, name, full_path.size() - dir_len - 1);
  full_path[full_path.size() - 1] = '\0';

  FILINFO fi_info;
  FRESULT fr = f_stat(full_path.data(), &fi_info);
  if (fr != FR_OK) {
    return std::unexpected(Error{FresultToErrorCode(fr)});
  }

  auto inode_result = fs_->AllocateInode();
  if (!inode_result) {
    return std::unexpected(inode_result.error());
  }
  vfs::Inode* inode = *inode_result;
  auto* new_fi = static_cast<FatInode*>(inode->fs_private);
  inode->ino = 0;
  inode->type = FilInfoToFileType(fi_info);
  inode->size = fi_info.fsize;
  inode->permissions = kDefaultFilePermissions;
  inode->link_count = 1;
  strncpy(new_fi->path.data(), full_path.data(), new_fi->path.size() - 1);
  new_fi->path[new_fi->path.size() - 1] = '\0';
  return inode;
}

auto FatFsFileSystem::FatFsInodeOps::Create(vfs::Inode* dir, const char* name,
                                            vfs::FileType type)
    -> Expected<vfs::Inode*> {
  auto* dir_fi = static_cast<FatInode*>(dir->fs_private);
  std::array<char, kPathBufSize> full_path{};
  strncpy(full_path.data(), dir_fi->path.data(), full_path.size() - 1);
  size_t dir_len = strlen(full_path.data());
  strncpy(full_path.data() + dir_len, name, full_path.size() - dir_len - 1);
  full_path[full_path.size() - 1] = '\0';

  if (type == vfs::FileType::kDirectory) {
    FRESULT fr = f_mkdir(full_path.data());
    if (fr != FR_OK && fr != FR_EXIST) {
      return std::unexpected(Error{FresultToErrorCode(fr)});
    }
  } else {
    FIL fil;
    FRESULT fr = f_open(&fil, full_path.data(), FA_CREATE_NEW | FA_WRITE);
    if (fr != FR_OK) {
      return std::unexpected(Error{FresultToErrorCode(fr)});
    }
    (void)f_close(&fil);
  }

  auto inode_result = fs_->AllocateInode();
  if (!inode_result) {
    return std::unexpected(inode_result.error());
  }
  vfs::Inode* inode = *inode_result;
  auto* new_fi = static_cast<FatInode*>(inode->fs_private);
  inode->ino = 0;
  inode->type = type;
  inode->size = 0;
  inode->permissions = kDefaultFilePermissions;
  inode->link_count = 1;
  strncpy(new_fi->path.data(), full_path.data(), new_fi->path.size() - 1);
  new_fi->path[new_fi->path.size() - 1] = '\0';
  return inode;
}

auto FatFsFileSystem::FatFsInodeOps::Unlink(vfs::Inode* dir, const char* name)
    -> Expected<void> {
  auto* dir_fi = static_cast<FatInode*>(dir->fs_private);
  std::array<char, kPathBufSize> full_path{};
  strncpy(full_path.data(), dir_fi->path.data(), full_path.size() - 1);
  size_t dir_len = strlen(full_path.data());
  strncpy(full_path.data() + dir_len, name, full_path.size() - dir_len - 1);
  full_path[full_path.size() - 1] = '\0';
  return FresultToExpected(f_unlink(full_path.data()));
}

auto FatFsFileSystem::FatFsInodeOps::Mkdir(vfs::Inode* dir, const char* name)
    -> Expected<vfs::Inode*> {
  return Create(dir, name, vfs::FileType::kDirectory);
}

auto FatFsFileSystem::FatFsInodeOps::Rmdir(vfs::Inode* dir, const char* name)
    -> Expected<void> {
  // f_unlink 同时处理文件和空目录
  return Unlink(dir, name);
}

auto FatFsFileSystem::FatFsFileOps::Open(vfs::File* file) -> Expected<void> {
  if (file->inode->type == vfs::FileType::kDirectory) {
    return {};
  }
  return fs_->OpenFil(file->inode, file->flags);
}

auto FatFsFileSystem::FatFsFileOps::Read(vfs::File* file, void* buf,
                                         size_t count) -> Expected<size_t> {
  auto* fi = static_cast<FatInode*>(file->inode->fs_private);
  if (fi->fil == nullptr) {
    return std::unexpected(Error{ErrorCode::kFsInvalidFd});
  }
  UINT bytes_read = 0;
  FRESULT fr = f_read(fi->fil, buf, static_cast<UINT>(count), &bytes_read);
  if (fr != FR_OK) {
    klog::Err("FatFsFileOps::Read: f_read failed ({})", static_cast<int>(fr));
    return std::unexpected(Error{FresultToErrorCode(fr)});
  }
  file->offset += bytes_read;
  return static_cast<size_t>(bytes_read);
}

auto FatFsFileSystem::FatFsFileOps::Write(vfs::File* file, const void* buf,
                                          size_t count) -> Expected<size_t> {
  auto* fi = static_cast<FatInode*>(file->inode->fs_private);
  if (fi->fil == nullptr) {
    return std::unexpected(Error{ErrorCode::kFsInvalidFd});
  }
  UINT bytes_written = 0;
  FRESULT fr = f_write(fi->fil, buf, static_cast<UINT>(count), &bytes_written);
  if (fr != FR_OK) {
    klog::Err("FatFsFileOps::Write: f_write failed ({})", static_cast<int>(fr));
    return std::unexpected(Error{FresultToErrorCode(fr)});
  }
  file->offset += bytes_written;
  file->inode->size = static_cast<uint64_t>(f_size(fi->fil));
  return static_cast<size_t>(bytes_written);
}

auto FatFsFileSystem::FatFsFileOps::Seek(vfs::File* file, int64_t offset,
                                         vfs::SeekWhence whence)
    -> Expected<uint64_t> {
  auto* fi = static_cast<FatInode*>(file->inode->fs_private);
  if (fi->fil == nullptr) {
    return std::unexpected(Error{ErrorCode::kFsInvalidFd});
  }
  FSIZE_t new_pos = 0;
  switch (whence) {
    case vfs::SeekWhence::kSet:
      new_pos = static_cast<FSIZE_t>(offset);
      break;
    case vfs::SeekWhence::kCur:
      new_pos =
          static_cast<FSIZE_t>(static_cast<int64_t>(f_tell(fi->fil)) + offset);
      break;
    case vfs::SeekWhence::kEnd:
      new_pos =
          static_cast<FSIZE_t>(static_cast<int64_t>(f_size(fi->fil)) + offset);
      break;
  }
  FRESULT fr = f_lseek(fi->fil, new_pos);
  if (fr != FR_OK) {
    return std::unexpected(Error{FresultToErrorCode(fr)});
  }
  file->offset = static_cast<uint64_t>(new_pos);
  return static_cast<uint64_t>(new_pos);
}

auto FatFsFileSystem::FatFsFileOps::Close(vfs::File* file) -> Expected<void> {
  auto* fi = static_cast<FatInode*>(file->inode->fs_private);

  // 如果有打开的 DIR，关闭并释放
  if (fi->dir != nullptr) {
    (void)f_closedir(fi->dir);
    fs_->FreeDir(fi->dir);
    fi->dir = nullptr;
  }

  // 关闭文件句柄（如果有）
  if (fi->fil == nullptr) {
    return {};
  }
  FRESULT fr = f_close(fi->fil);
  fs_->FreeFil(fi->fil);
  fi->fil = nullptr;
  return FresultToExpected(fr);
}

auto FatFsFileSystem::FatFsFileOps::ReadDir(vfs::File* file,
                                            vfs::DirEntry* dirent, size_t count)
    -> Expected<size_t> {
  auto* fi = static_cast<FatInode*>(file->inode->fs_private);

  // 首次调用时惰性初始化 DIR（保持迭代状态）
  if (fi->dir == nullptr) {
    DIR* dir = fs_->AllocateDir();
    if (dir == nullptr) {
      return std::unexpected(Error{ErrorCode::kFsFdTableFull});
    }
    FRESULT fr = f_opendir(dir, fi->path.data());
    if (fr != FR_OK) {
      fs_->FreeDir(dir);
      return std::unexpected(Error{FresultToErrorCode(fr)});
    }
    fi->dir = dir;
  }

  size_t read_count = 0;
  for (size_t i = 0; i < count; ++i) {
    FILINFO fi_info;
    FRESULT fr = f_readdir(fi->dir, &fi_info);
    if (fr != FR_OK || fi_info.fname[0] == '\0') {
      break;
    }
    dirent[i].ino = 0;
    dirent[i].type = static_cast<uint8_t>((fi_info.fattrib & AM_DIR) != 0
                                              ? vfs::FileType::kDirectory
                                              : vfs::FileType::kRegular);
    strncpy(dirent[i].name, fi_info.fname, sizeof(dirent[i].name) - 1);
    dirent[i].name[sizeof(dirent[i].name) - 1] = '\0';
    ++read_count;
  }
  file->offset += static_cast<uint64_t>(read_count);
  return read_count;
}

}  // namespace fatfs

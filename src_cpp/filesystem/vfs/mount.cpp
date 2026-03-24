/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "mount.hpp"

#include "kernel_log.hpp"
#include "kstd_cstring"
#include "kstd_memory"
#include "spinlock.hpp"
#include "vfs.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto MountTable::Mount(const char* path, FileSystem* fs, BlockDevice* device)
    -> Expected<void> {
  if (path == nullptr || fs == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  // 检查挂载点数量
  if (mount_count_ >= kMaxMounts) {
    return std::unexpected(Error(ErrorCode::kFsMountFailed));
  }

  // 规范化路径（确保以 / 开头）
  if (path[0] != '/') {
    return std::unexpected(Error(ErrorCode::kFsInvalidPath));
  }

  // 检查路径是否已被挂载
  for (size_t i = 0; i < mount_count_; ++i) {
    if (mounts_[i].active && strcmp(mounts_[i].mount_path, path) == 0) {
      return std::unexpected(Error(ErrorCode::kFsAlreadyMounted));
    }
  }

  // 挂载文件系统
  auto mount_result = fs->Mount(device);
  if (!mount_result.has_value()) {
    klog::Err("MountTable: failed to mount filesystem '{}': {}", fs->GetName(),
              mount_result.error().message());
    return std::unexpected(Error(ErrorCode::kFsMountFailed));
  }

  Inode* root_inode = mount_result.value();
  if (root_inode == nullptr || root_inode->type != FileType::kDirectory) {
    return std::unexpected(Error(ErrorCode::kFsCorrupted));
  }

  // 为根 inode 创建 dentry
  auto root_dentry_ptr = kstd::make_unique<Dentry>();
  if (!root_dentry_ptr) {
    fs->Unmount();
    return std::unexpected(Error(ErrorCode::kOutOfMemory));
  }

  root_dentry_ptr->inode = root_inode;
  strncpy(root_dentry_ptr->name, "/", sizeof(root_dentry_ptr->name));

  // 查找挂载点 dentry（如果是非根挂载）
  Dentry* mount_dentry = nullptr;
  if (strcmp(path, "/") != 0) {
    // 需要找到父文件系统中对应的 dentry
    // 这里简化处理，实际应该通过 VFS Lookup 找到
    // 暂时设置为 nullptr，表示根挂载
  }

  // 找到空闲挂载点槽位
  size_t slot = 0;
  for (; slot < kMaxMounts; ++slot) {
    if (!mounts_[slot].active) {
      break;
    }
  }

  if (slot >= kMaxMounts) {
    // root_dentry_ptr 自动释放（RAII）
    fs->Unmount();
    return std::unexpected(Error(ErrorCode::kFsMountFailed));
  }

  // 填充挂载点信息
  mounts_[slot].mount_path = path;
  mounts_[slot].mount_dentry = mount_dentry;
  mounts_[slot].filesystem = fs;
  mounts_[slot].device = device;
  mounts_[slot].root_inode = root_inode;
  mounts_[slot].root_dentry = root_dentry_ptr.release();
  mounts_[slot].active = true;

  ++mount_count_;

  // 如果是根挂载，设置 root_mount_
  if (strcmp(path, "/") == 0) {
    root_mount_ = &mounts_[slot];
    // 更新 VFS 根 dentry
    extern void SetRootDentry(Dentry*);
    SetRootDentry(mounts_[slot].root_dentry);
  }

  klog::Info("MountTable: mounted '{}' on '{}'", fs->GetName(), path);
  return {};
}

auto MountTable::Unmount(const char* path) -> Expected<void> {
  if (path == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  // 查找挂载点
  MountPoint* mp = nullptr;
  size_t mp_index = 0;
  for (size_t i = 0; i < kMaxMounts; ++i) {
    if (mounts_[i].active && strcmp(mounts_[i].mount_path, path) == 0) {
      mp = &mounts_[i];
      mp_index = i;
      break;
    }
  }

  if (mp == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsNotMounted));
  }

  // 卸载文件系统
  auto result = mp->filesystem->Unmount();
  if (!result.has_value()) {
    return std::unexpected(result.error());
  }

  // 清理挂载点（RAII）
  etl::unique_ptr<Dentry> root_guard(mp->root_dentry);

  mp->active = false;
  mp->mount_path = nullptr;
  mp->mount_dentry = nullptr;
  mp->filesystem = nullptr;
  mp->device = nullptr;
  mp->root_inode = nullptr;
  mp->root_dentry = nullptr;

  --mount_count_;

  // 如果是根挂载，清除 root_mount_
  if (root_mount_ == &mounts_[mp_index]) {
    root_mount_ = nullptr;
    extern void SetRootDentry(Dentry*);
    SetRootDentry(nullptr);
  }

  klog::Info("MountTable: unmounted '{}'", path);
  return {};
}

auto MountTable::Lookup(const char* path) -> MountPoint* {
  if (path == nullptr || path[0] != '/') {
    return nullptr;
  }

  MountPoint* best_match = nullptr;
  size_t best_match_len = 0;

  for (size_t i = 0; i < kMaxMounts; ++i) {
    if (!mounts_[i].active) {
      continue;
    }

    const char* mp_path = mounts_[i].mount_path;
    if (mp_path == nullptr) {
      continue;
    }

    size_t mp_len = strlen(mp_path);

    // 检查路径是否以挂载路径开头
    if (strncmp(path, mp_path, mp_len) == 0) {
      // 确保是完整匹配或下一个字符是 /，或者是根目录挂载
      char next_char = path[mp_len];
      if (next_char == '\0' || next_char == '/' ||
          (mp_len == 1 && mp_path[0] == '/')) {
        // 选择最长的匹配
        if (mp_len > best_match_len) {
          best_match = &mounts_[i];
          best_match_len = mp_len;
        }
      }
    }
  }

  return best_match;
}

auto MountTable::GetRootDentry(MountPoint* mp) -> Dentry* {
  if (mp == nullptr || !mp->active) {
    return nullptr;
  }
  return mp->root_dentry;
}

auto MountTable::FindByMountDentry(const Dentry* dentry) -> MountPoint* {
  if (dentry == nullptr) {
    return nullptr;
  }

  for (size_t i = 0; i < kMaxMounts; ++i) {
    if (mounts_[i].active && mounts_[i].mount_dentry == dentry) {
      return &mounts_[i];
    }
  }

  return nullptr;
}

auto MountTable::IsMountPoint(const char* path) -> bool {
  if (path == nullptr) {
    return false;
  }

  for (size_t i = 0; i < kMaxMounts; ++i) {
    if (mounts_[i].active && mounts_[i].mount_path != nullptr &&
        strcmp(mounts_[i].mount_path, path) == 0) {
      return true;
    }
  }

  return false;
}

auto MountTable::GetRootMount() -> MountPoint* { return root_mount_; }

auto GetMountTable() -> MountTable& {
  static MountTable instance;
  return instance;
}

}  // namespace vfs

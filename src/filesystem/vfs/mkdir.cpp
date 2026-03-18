/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "kstd_memory"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto MkDir(const char* path) -> Expected<void> {
  if (path == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  // 解析父目录路径和目录名
  char parent_path[512];
  char dir_name[256];
  const char* last_slash = strrchr(path, '/');
  if (last_slash == nullptr || last_slash == path) {
    strncpy(parent_path, "/", sizeof(parent_path));
    const char* name_start = path[0] == '/' ? path + 1 : path;
    if (strlen(name_start) >= sizeof(dir_name)) {
      return std::unexpected(Error(ErrorCode::kFsInvalidPath));
    }
    strncpy(dir_name, name_start, sizeof(dir_name));
  } else {
    size_t parent_len = last_slash - path;
    if (parent_len >= sizeof(parent_path)) {
      return std::unexpected(Error(ErrorCode::kFsInvalidPath));
    }
    strncpy(parent_path, path, parent_len);
    parent_path[parent_len] = '\0';
    if (strlen(last_slash + 1) >= sizeof(dir_name)) {
      return std::unexpected(Error(ErrorCode::kFsInvalidPath));
    }
    strncpy(dir_name, last_slash + 1, sizeof(dir_name));
  }
  dir_name[sizeof(dir_name) - 1] = '\0';

  // 查找父目录
  auto parent_result = Lookup(parent_path);
  if (!parent_result.has_value()) {
    return std::unexpected(parent_result.error());
  }

  Dentry* parent_dentry = parent_result.value();
  if (parent_dentry->inode == nullptr ||
      parent_dentry->inode->type != FileType::kDirectory) {
    return std::unexpected(Error(ErrorCode::kFsNotADirectory));
  }

  // 检查目录是否已存在
  if (FindChild(parent_dentry, dir_name) != nullptr) {
    return std::unexpected(Error(ErrorCode::kFsFileExists));
  }

  // 创建目录
  if (parent_dentry->inode->ops == nullptr) {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }

  auto result =
      parent_dentry->inode->ops->Mkdir(parent_dentry->inode, dir_name);
  if (!result.has_value()) {
    return std::unexpected(result.error());
  }

  // 创建 dentry
  auto new_dentry = kstd::make_unique<Dentry>();
  if (!new_dentry) {
    return std::unexpected(Error(ErrorCode::kOutOfMemory));
  }

  strncpy(new_dentry->name, dir_name, sizeof(new_dentry->name) - 1);
  new_dentry->name[sizeof(new_dentry->name) - 1] = '\0';
  new_dentry->inode = result.value();
  AddChild(parent_dentry, new_dentry.release());

  klog::Debug("VFS: created directory '{}'", path);
  return {};
}

}  // namespace vfs

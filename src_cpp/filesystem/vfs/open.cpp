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

auto Open(const char* path, OpenFlags flags) -> Expected<File*> {
  if (!GetVfsState().initialized) {
    return std::unexpected(Error(ErrorCode::kFsNotMounted));
  }

  if (path == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  if (strlen(path) >= kMaxPathLength) {
    return std::unexpected(Error(ErrorCode::kFsInvalidPath));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  // 查找或创建 dentry
  auto lookup_result = Lookup(path);
  Dentry* dentry = nullptr;

  if (!lookup_result.has_value()) {
    // 文件不存在，检查是否需要创建
    if ((flags & OpenFlags::kOCreate) == 0U) {
      return std::unexpected(lookup_result.error());
    }

    // 创建新文件
    // 找到父目录路径
    char parent_path[512];
    char file_name[256];
    const char* last_slash = strrchr(path, '/');
    if (last_slash == nullptr || last_slash == path) {
      strncpy(parent_path, "/", sizeof(parent_path));
      const char* name_start = path[0] == '/' ? path + 1 : path;
      if (strlen(name_start) >= sizeof(file_name)) {
        return std::unexpected(Error(ErrorCode::kFsInvalidPath));
      }
      strncpy(file_name, name_start, sizeof(file_name));
    } else {
      size_t parent_len = last_slash - path;
      if (parent_len >= sizeof(parent_path)) {
        return std::unexpected(Error(ErrorCode::kFsInvalidPath));
      }
      strncpy(parent_path, path, parent_len);
      parent_path[parent_len] = '\0';
      if (strlen(last_slash + 1) >= sizeof(file_name)) {
        return std::unexpected(Error(ErrorCode::kFsInvalidPath));
      }
      strncpy(file_name, last_slash + 1, sizeof(file_name));
    }
    file_name[sizeof(file_name) - 1] = '\0';

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

    // 创建文件
    if (parent_dentry->inode->ops == nullptr) {
      return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
    }

    auto create_result = parent_dentry->inode->ops->Create(
        parent_dentry->inode, file_name, FileType::kRegular);
    if (!create_result.has_value()) {
      return std::unexpected(create_result.error());
    }

    // 创建 dentry
    auto new_dentry = kstd::make_unique<Dentry>();
    if (!new_dentry) {
      return std::unexpected(Error(ErrorCode::kOutOfMemory));
    }

    strncpy(new_dentry->name, file_name, sizeof(new_dentry->name) - 1);
    new_dentry->name[sizeof(new_dentry->name) - 1] = '\0';
    new_dentry->inode = create_result.value();
    dentry = new_dentry.get();
    AddChild(parent_dentry, new_dentry.release());
  } else {
    dentry = lookup_result.value();
  }

  if (dentry == nullptr || dentry->inode == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsCorrupted));
  }

  // 检查打开模式
  if ((flags & OpenFlags::kODirectory) != 0U &&
      dentry->inode->type != FileType::kDirectory) {
    return std::unexpected(Error(ErrorCode::kFsNotADirectory));
  }

  // 创建 File 对象
  auto new_file = kstd::make_unique<File>();
  if (!new_file) {
    return std::unexpected(Error(ErrorCode::kOutOfMemory));
  }
  File* file = new_file.release();

  file->inode = dentry->inode;
  file->dentry = dentry;
  file->offset = 0;
  file->flags = flags;
  dentry->ref_count++;

  // 从文件系统获取 FileOps
  if (file->inode != nullptr && file->inode->fs != nullptr) {
    file->ops = file->inode->fs->GetFileOps();
  }

  // 让文件系统准备底层 I/O 句柄（如 FatFS 的 FIL 对象）
  if (file->ops != nullptr) {
    auto open_result = file->ops->Open(file);
    if (!open_result.has_value()) {
      dentry->ref_count--;
      delete file;
      return std::unexpected(open_result.error());
    }
  }

  // 处理 O_TRUNC
  if ((flags & OpenFlags::kOTruncate) != 0U &&
      dentry->inode->type == FileType::kRegular) {
    // 截断文件，由具体文件系统处理
    // 这里不直接操作，而是通过后续的 write 来处理
  }

  klog::Debug("VFS: opened '{}', flags={:#x}", path,
              static_cast<uint32_t>(flags));
  return file;
}

}  // namespace vfs

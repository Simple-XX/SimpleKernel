/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto Seek(File* file, int64_t offset, SeekWhence whence) -> Expected<uint64_t> {
  if (file == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  if (file->ops != nullptr) {
    return file->ops->Seek(file, offset, whence);
  }

  // 默认实现
  uint64_t new_offset = file->offset;

  switch (whence) {
    case SeekWhence::kSet:
      if (offset < 0) {
        return std::unexpected(Error(ErrorCode::kInvalidArgument));
      }
      new_offset = static_cast<uint64_t>(offset);
      break;
    case SeekWhence::kCur:
      if (offset < 0 && static_cast<uint64_t>(-offset) > file->offset) {
        return std::unexpected(Error(ErrorCode::kInvalidArgument));
      }
      new_offset =
          static_cast<uint64_t>(static_cast<int64_t>(file->offset) + offset);
      break;
    case SeekWhence::kEnd:
      if (file->inode == nullptr) {
        return std::unexpected(Error(ErrorCode::kFsCorrupted));
      }
      if (offset < 0 && static_cast<uint64_t>(-offset) > file->inode->size) {
        return std::unexpected(Error(ErrorCode::kInvalidArgument));
      }
      new_offset = static_cast<uint64_t>(
          static_cast<int64_t>(file->inode->size) + offset);
      break;
    default:
      return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  file->offset = new_offset;
  return new_offset;
}

}  // namespace vfs

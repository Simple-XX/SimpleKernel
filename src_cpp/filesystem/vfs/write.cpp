/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto Write(File* file, const void* buf, size_t count) -> Expected<size_t> {
  if (file == nullptr || buf == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  // 检查写入权限
  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  if ((file->flags & OpenFlags::kOWriteOnly) == 0U &&
      (file->flags & OpenFlags::kOReadWrite) == 0U) {
    return std::unexpected(Error(ErrorCode::kFsPermissionDenied));
  }

  if (file->ops == nullptr) {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }

  return file->ops->Write(file, buf, count);
}

}  // namespace vfs

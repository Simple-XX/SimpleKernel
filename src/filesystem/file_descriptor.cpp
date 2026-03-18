/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "file_descriptor.hpp"

#include "kernel_log.hpp"

namespace filesystem {

FileDescriptorTable::FileDescriptorTable()
    : table_{}, open_count_(0), lock_{"fd_table"} {}

FileDescriptorTable::~FileDescriptorTable() {
  CloseAll().or_else([](auto&& err) {
    klog::Warn("Failed to close all files in destructor: {}", err.message());
    return Expected<void>{};
  });
}

FileDescriptorTable::FileDescriptorTable(FileDescriptorTable&& other)
    : open_count_(other.open_count_), lock_{"fd_table"} {
  LockGuard guard(other.lock_);

  for (int i = 0; i < kMaxFd; ++i) {
    table_[i] = other.table_[i];
    other.table_[i] = nullptr;
  }

  other.open_count_ = 0;
}

auto FileDescriptorTable::operator=(FileDescriptorTable&& other)
    -> FileDescriptorTable& {
  if (this != &other) {
    // 先关闭当前的所有文件
    CloseAll().or_else([](auto&& err) {
      klog::Warn("Failed to close all files in move assignment: {}",
                 err.message());
      return Expected<void>{};
    });

    // 使用地址确定加锁顺序，避免死锁
    auto* first_lock = (this < &other) ? &lock_ : &other.lock_;
    auto* second_lock = (this < &other) ? &other.lock_ : &lock_;
    LockGuard guard1(*first_lock);
    LockGuard guard2(*second_lock);

    for (int i = 0; i < kMaxFd; ++i) {
      table_[i] = other.table_[i];
      other.table_[i] = nullptr;
    }

    open_count_ = other.open_count_;
    other.open_count_ = 0;
  }

  return *this;
}

auto FileDescriptorTable::Alloc(vfs::File* file) -> Expected<int> {
  if (file == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard guard(lock_);

  // 从 3 开始查找（0/1/2 预留给标准流）
  for (int fd = 3; fd < kMaxFd; ++fd) {
    if (table_[fd] == nullptr) {
      table_[fd] = file;
      ++open_count_;
      return fd;
    }
  }

  return std::unexpected(Error(ErrorCode::kFsFdTableFull));
}

auto FileDescriptorTable::Get(int fd) -> vfs::File* {
  if (fd < 0 || fd >= kMaxFd) {
    return nullptr;
  }

  LockGuard guard(lock_);
  return table_[fd];
}

auto FileDescriptorTable::Free(int fd) -> Expected<void> {
  if (fd < 0 || fd >= kMaxFd) {
    return std::unexpected(Error(ErrorCode::kFsInvalidFd));
  }

  LockGuard guard(lock_);

  if (table_[fd] == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsInvalidFd));
  }

  table_[fd] = nullptr;
  --open_count_;

  return {};
}

auto FileDescriptorTable::Dup(int old_fd, int new_fd) -> Expected<int> {
  if (old_fd < 0 || old_fd >= kMaxFd) {
    return std::unexpected(Error(ErrorCode::kFsInvalidFd));
  }

  LockGuard guard(lock_);

  vfs::File* file = table_[old_fd];
  if (file == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsInvalidFd));
  }

  if (new_fd >= 0 && new_fd < kMaxFd) {
    // 关闭目标 fd 如果已打开
    if (table_[new_fd] != nullptr) {
      table_[new_fd] = nullptr;
      --open_count_;
    }

    table_[new_fd] = file;
    ++open_count_;
    return new_fd;
  }

  // 分配新的 fd
  if (new_fd == -1) {
    for (int fd = kStderrFd + 1; fd < kMaxFd; ++fd) {
      if (table_[fd] == nullptr) {
        table_[fd] = file;
        ++open_count_;
        return fd;
      }
    }
  }

  return std::unexpected(Error(ErrorCode::kFsFdTableFull));
}

auto FileDescriptorTable::CloseAll() -> Expected<void> {
  LockGuard guard(lock_);

  for (int fd = 0; fd < kMaxFd; ++fd) {
    if (table_[fd] != nullptr) {
      // 注意：这里不调用 VFS Close，因为 File 对象可能在其他地方共享
      // 实际关闭操作由调用者负责
      table_[fd] = nullptr;
    }
  }

  open_count_ = 0;
  return {};
}

auto FileDescriptorTable::SetupStandardFiles(vfs::File* stdin_file,
                                             vfs::File* stdout_file,
                                             vfs::File* stderr_file)
    -> Expected<void> {
  LockGuard guard(lock_);

  // 仅对之前未占用的槽位增加计数
  if (table_[kStdinFd] == nullptr && stdin_file != nullptr) {
    ++open_count_;
  }
  if (table_[kStdoutFd] == nullptr && stdout_file != nullptr) {
    ++open_count_;
  }
  if (table_[kStderrFd] == nullptr && stderr_file != nullptr) {
    ++open_count_;
  }

  table_[kStdinFd] = stdin_file;
  table_[kStdoutFd] = stdout_file;
  table_[kStderrFd] = stderr_file;

  return {};
}

auto FileDescriptorTable::GetOpenCount() const -> int { return open_count_; }

}  // namespace filesystem

/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "expected.hpp"
#include "spinlock.hpp"
#include "vfs.hpp"

namespace filesystem {

/**
 * @brief 进程级文件描述符表
 * @details 每个进程（TaskControlBlock）持有一个 FdTable，
 *          将整数 fd 映射到 File 对象。
 *          fd 0/1/2 预留给 stdin/stdout/stderr。
 */
class FileDescriptorTable {
 public:
  /// 最大文件描述符数
  static constexpr int kMaxFd = 64;

  /// 标准文件描述符
  static constexpr int kStdinFd = 0;
  static constexpr int kStdoutFd = 1;
  static constexpr int kStderrFd = 2;

  /// @name 构造/析构函数
  /// @{
  FileDescriptorTable();
  FileDescriptorTable(const FileDescriptorTable&) = delete;
  FileDescriptorTable(FileDescriptorTable&& other);
  auto operator=(const FileDescriptorTable&) -> FileDescriptorTable& = delete;
  auto operator=(FileDescriptorTable&& other) -> FileDescriptorTable&;
  ~FileDescriptorTable();
  /// @}

  /**
   * @brief 分配一个最小可用 fd 并关联 File
   * @param file 要关联的 File 对象
   * @return Expected<int> 分配到的 fd
   * @post 返回的 fd >= 0 且 fd < kMaxFd
   */
  [[nodiscard]] auto Alloc(vfs::File* file) -> Expected<int>;

  /**
   * @brief 获取 fd 对应的 File 对象
   * @param fd 文件描述符
   * @return vfs::File* 指针，无效 fd 返回 nullptr
   * @pre 0 <= fd < kMaxFd
   */
  [[nodiscard]] auto Get(int fd) -> vfs::File*;

  /**
   * @brief 释放 fd
   * @param fd 要释放的文件描述符
   * @return Expected<void>
   */

  [[nodiscard]] auto Free(int fd) -> Expected<void>;
  /**
   * @brief 复制文件描述符（用于 dup/dup2）
   * @param old_fd 原文件描述符
   * @param new_fd 目标文件描述符（若为 -1 则分配最小可用）
   * @return Expected<int> 新的文件描述符
   */

  [[nodiscard]] auto Dup(int old_fd, int new_fd = -1) -> Expected<int>;
  /**
   * @brief 关闭所有文件描述符
   * @return Expected<void> 成功或错误
   */

  [[nodiscard]] auto CloseAll() -> Expected<void>;

  /**
   * @brief 设置标准文件描述符
   * @param stdin_file stdin 文件对象
   * @param stdout_file stdout 文件对象
   * @param stderr_file stderr 文件对象
   * @return Expected<void> 成功或错误
   */
  [[nodiscard]] auto SetupStandardFiles(vfs::File* stdin_file,
                                        vfs::File* stdout_file,
                                        vfs::File* stderr_file)
      -> Expected<void>;
  /**
   * @brief 获取已打开文件描述符数量
   * @return int 已打开 fd 数量
   */
  [[nodiscard]] auto GetOpenCount() const -> int;

 private:
  std::array<vfs::File*, kMaxFd> table_;
  int open_count_{0};
  SpinLock lock_{"fd_table"};
};

}  // namespace filesystem

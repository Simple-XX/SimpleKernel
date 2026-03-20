/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "expected.hpp"

namespace vfs {

// Forward declarations
struct Inode;
struct Dentry;
struct File;
struct FileSystem;
class MountTable;

/// 文件类型
enum class FileType : uint8_t {
  kUnknown = 0,
  /// 普通文件
  kRegular = 1,
  /// 目录
  kDirectory = 2,
  /// 字符设备
  kCharDevice = 3,
  /// 块设备
  kBlockDevice = 4,
  /// 符号链接
  kSymlink = 5,
  /// 命名管道
  kFifo = 6,
};

/// 文件打开标志（兼容 Linux O_* 定义）
enum class OpenFlags : uint32_t {
  kOReadOnly = 0x0000,
  kOWriteOnly = 0x0001,
  kOReadWrite = 0x0002,
  kOCreate = 0x0040,
  kOTruncate = 0x0200,
  kOAppend = 0x0400,
  /// 必须是目录
  kODirectory = 0x010000,
};

/// 按位或
[[nodiscard]] inline constexpr auto operator|(OpenFlags lhs, OpenFlags rhs)
    -> OpenFlags {
  return static_cast<OpenFlags>(static_cast<uint32_t>(lhs) |
                                static_cast<uint32_t>(rhs));
}

/// 按位与
[[nodiscard]] inline constexpr auto operator&(OpenFlags lhs, OpenFlags rhs)
    -> OpenFlags {
  return static_cast<OpenFlags>(static_cast<uint32_t>(lhs) &
                                static_cast<uint32_t>(rhs));
}

/// 按位取反
[[nodiscard]] inline constexpr auto operator~(OpenFlags flags) -> OpenFlags {
  return static_cast<OpenFlags>(~static_cast<uint32_t>(flags));
}

/// 按位或赋值
inline constexpr auto operator|=(OpenFlags& lhs, OpenFlags rhs) -> OpenFlags& {
  lhs = lhs | rhs;
  return lhs;
}

/// 按位与赋值
inline constexpr auto operator&=(OpenFlags& lhs, OpenFlags rhs) -> OpenFlags& {
  lhs = lhs & rhs;
  return lhs;
}

/// 检查 OpenFlags 是否为零（无标志位设置）
[[nodiscard]] inline constexpr auto operator==(OpenFlags flags, uint32_t val)
    -> bool {
  return static_cast<uint32_t>(flags) == val;
}

/// 检查 OpenFlags 是否不为零
[[nodiscard]] inline constexpr auto operator!=(OpenFlags flags, uint32_t val)
    -> bool {
  return static_cast<uint32_t>(flags) != val;
}

/// 文件 seek 基准
enum class SeekWhence : int {
  /// 从文件开头
  kSet = 0,
  /// 从当前位置
  kCur = 1,
  /// 从文件末尾
  kEnd = 2,
};

/// Inode 操作接口
class InodeOps {
 public:
  /**
   * @brief 在目录中查找指定名称的 inode
   * @param dir 父目录 inode
   * @param name 要查找的文件名
   * @return Expected<Inode*> 找到的 inode 或错误
   * @pre dir != nullptr && dir->type == FileType::kDirectory
   * @pre name != nullptr && strlen(name) > 0
   */
  virtual auto Lookup(Inode* dir, const char* name) -> Expected<Inode*> = 0;

  /**
   * @brief 在目录中创建新文件
   * @param dir 父目录 inode
   * @param name 文件名
   * @param type 文件类型
   * @return Expected<Inode*> 新创建的 inode 或错误
   * @pre dir != nullptr && dir->type == FileType::kDirectory
   * @pre name != nullptr && strlen(name) > 0
   * @post 返回的 inode->type == type
   */
  virtual auto Create(Inode* dir, const char* name, FileType type)
      -> Expected<Inode*> = 0;

  /**
   * @brief 删除文件（解除链接）
   * @param dir 父目录 inode
   * @param name 要删除的文件名
   * @return Expected<void> 成功或错误
   * @pre dir != nullptr && dir->type == FileType::kDirectory
   * @pre name != nullptr
   */
  virtual auto Unlink(Inode* dir, const char* name) -> Expected<void> = 0;

  /**
   * @brief 创建目录
   * @param dir 父目录 inode
   * @param name 目录名
   * @return Expected<Inode*> 新创建的目录 inode 或错误
   * @pre dir != nullptr && dir->type == FileType::kDirectory
   * @pre name != nullptr && strlen(name) > 0
   * @post 返回的 inode->type == FileType::kDirectory
   */
  virtual auto Mkdir(Inode* dir, const char* name) -> Expected<Inode*> = 0;

  /**
   * @brief 删除目录
   * @param dir 父目录 inode
   * @param name 要删除的目录名
   * @return Expected<void> 成功或错误
   * @pre dir != nullptr && dir->type == FileType::kDirectory
   * @pre name != nullptr
   */
  virtual auto Rmdir(Inode* dir, const char* name) -> Expected<void> = 0;

  /// @name 构造/析构函数
  /// @{
  InodeOps() = default;
  InodeOps(const InodeOps&) = delete;
  InodeOps(InodeOps&&) = delete;
  auto operator=(const InodeOps&) -> InodeOps& = delete;
  auto operator=(InodeOps&&) -> InodeOps& = delete;
  virtual ~InodeOps() = default;
  /// @}
};

/// 目录项结构（用于 readdir）
struct DirEntry {
  /// inode 编号
  uint64_t ino{0};
  /// 文件类型
  uint8_t type{0};
  /// 文件名
  char name[256]{};
};

/// File 操作接口
class FileOps {
 public:
  /**
   * @brief 打开文件，准备底层 I/O 句柄
   * @param file 文件对象
   * @return Expected<void> 成功或错误
   * @pre file != nullptr
   * @note 默认实现为空操作，适用于不需要额外打开步骤的文件系统（如 ramfs）
   */
  virtual auto Open(File* /*file*/) -> Expected<void> { return {}; }

  /**
   * @brief 从文件读取数据
   * @param file 文件对象
   * @param buf 输出缓冲区
   * @param count 最大读取字节数
   * @return Expected<size_t> 实际读取的字节数或错误
   * @pre file != nullptr && buf != nullptr
   * @post 返回值 <= count
   */
  virtual auto Read(File* file, void* buf, size_t count)
      -> Expected<size_t> = 0;

  /**
   * @brief 向文件写入数据
   * @param file 文件对象
   * @param buf 输入缓冲区
   * @param count 要写入的字节数
   * @return Expected<size_t> 实际写入的字节数或错误
   * @pre file != nullptr && buf != nullptr
   * @post 返回值 <= count
   */
  virtual auto Write(File* file, const void* buf, size_t count)
      -> Expected<size_t> = 0;

  /**
   * @brief 调整文件偏移量
   * @param file 文件对象
   * @param offset 偏移量
   * @param whence 基准位置
   * @return Expected<uint64_t> 新的偏移量或错误
   * @pre file != nullptr
   */
  virtual auto Seek(File* file, int64_t offset, SeekWhence whence)
      -> Expected<uint64_t> = 0;

  /**
   * @brief 关闭文件
   * @param file 文件对象
   * @return Expected<void> 成功或错误
   * @pre file != nullptr
   * @post file 对象将被释放
   */
  virtual auto Close(File* file) -> Expected<void> = 0;

  /**
   * @brief 读取目录项
   * @param file 目录文件对象
   * @param dirent 输出目录项数组
   * @param count 最多读取的条目数
   * @return Expected<size_t> 实际读取的条目数或错误
   * @pre file != nullptr && file->inode->type == FileType::kDirectory
   * @pre dirent != nullptr
   */
  virtual auto ReadDir(File* file, DirEntry* dirent, size_t count)
      -> Expected<size_t> = 0;

  /// @name 构造/析构函数
  /// @{
  FileOps() = default;
  FileOps(const FileOps&) = delete;
  FileOps(FileOps&&) = delete;
  auto operator=(const FileOps&) -> FileOps& = delete;
  auto operator=(FileOps&&) -> FileOps& = delete;
  virtual ~FileOps() = default;
  /// @}
};

}  // namespace vfs

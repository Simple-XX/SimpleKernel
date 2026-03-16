/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include "vfs_types.hpp"

namespace vfs {

/**
 * @brief Inode — 文件元数据（独立于路径名）
 * @details 每个文件/目录在 VFS 中有且仅有一个 Inode。
 *          Inode 持有文件的元信息和操作方法指针。
 */
struct Inode {
  /// inode 编号（文件系统内唯一）
  uint64_t ino{0};
  /// 文件类型
  FileType type{FileType::kUnknown};
  /// 文件大小（字节）
  uint64_t size{0};
  /// 权限位（简化版）
  uint32_t permissions{0644};
  /// 硬链接计数
  uint32_t link_count{1};
  /// 文件系统私有数据指针
  void* fs_private{nullptr};
  /// 所属文件系统
  FileSystem* fs{nullptr};

  /// 文件操作接口
  InodeOps* ops{nullptr};
};

/**
 * @brief Dentry — 目录项缓存（路径名 ↔ Inode 的映射）
 * @details Dentry 构成一棵树，反映目录层次结构。
 *          支持路径查找加速。
 */
struct Dentry {
  /// 文件/目录名
  char name[256]{};
  /// 关联的 inode
  Inode* inode{nullptr};
  /// 父目录项
  Dentry* parent{nullptr};
  /// 子目录项链表头
  Dentry* children{nullptr};
  /// 兄弟目录项（同一父目录下）
  Dentry* next_sibling{nullptr};
  /// 文件系统私有数据
  void* fs_private{nullptr};
  /// 引用计数（打开的 File 数量）
  uint32_t ref_count{0};
  /// 标记为已删除（unlink/rmdir 时 ref_count > 0）
  bool deleted{false};
};

/**
 * @brief File — 打开的文件实例（每次 open 产生一个）
 * @details File 对象持有当前偏移量和操作方法指针。
 *          多个 File 可以指向同一个 Inode。
 */
struct File {
  /// 关联的 inode
  Inode* inode{nullptr};
  /// 关联的 dentry
  Dentry* dentry{nullptr};
  /// 当前读写偏移量
  uint64_t offset{0};
  /// 打开标志 (OpenFlags)
  OpenFlags flags{OpenFlags::kOReadOnly};

  /// 文件操作接口
  FileOps* ops{nullptr};
};

/**
 * @brief VFS 全局初始化
 * @return Expected<void> 成功或错误
 * @post VFS 子系统已准备好接受挂载请求
 * @note 线程安全：应在系统启动时单线程调用
 * @note 幂等性：重复调用会返回成功（无操作）
 */
[[nodiscard]] auto Init() -> Expected<void>;

/**
 * @brief 路径解析，查找 dentry
 * @param path 绝对路径（以 / 开头）
 * @return Expected<Dentry*> 找到的 dentry 或错误
 * @pre path != nullptr && path[0] == '/'
 * @note 线程安全：是，内部使用 MountTable 锁
 * @note 会自动跨越挂载点解析路径
 */
[[nodiscard]] auto Lookup(const char* path) -> Expected<Dentry*>;

/**
 * @brief 打开文件
 * @param path 文件路径
 * @param flags 打开标志
 * @return Expected<File*> 文件对象或错误
 * @pre path != nullptr
 * @post 成功时返回有效的 File 对象
 * @note 调用者负责调用 Close() 释放 File 对象
 * @note 若 flags 包含 kOCreate，文件不存在时会自动创建
 */
[[nodiscard]] auto Open(const char* path, OpenFlags flags) -> Expected<File*>;

/**
 * @brief 关闭文件
 * @param file 文件对象
 * @return Expected<void> 成功或错误
 * @pre file != nullptr
 * @note 会调用文件系统特定的 close 回调
 * @note 关闭后 File 指针失效，不应再使用
 */
[[nodiscard]] auto Close(File* file) -> Expected<void>;

/**
 * @brief 从文件读取数据
 * @param file 文件对象
 * @param buf 输出缓冲区
 * @param count 最大读取字节数
 * @return Expected<size_t> 实际读取的字节数或错误
 * @pre file != nullptr && buf != nullptr
 * @note 实际读取字节数可能小于 count（到达文件末尾）
 * @note 会自动更新 file->offset
 */
[[nodiscard]] auto Read(File* file, void* buf, size_t count)
    -> Expected<size_t>;

/**
 * @brief 向文件写入数据
 * @param file 文件对象
 * @param buf 输入缓冲区
 * @param count 要写入的字节数
 * @return Expected<size_t> 实际写入的字节数或错误
 * @pre file != nullptr && buf != nullptr
 * @note 文件系统可能需要在写入前扩展文件大小
 * @note 会自动更新 file->offset 和 file->inode->size
 */
[[nodiscard]] auto Write(File* file, const void* buf, size_t count)
    -> Expected<size_t>;

/**
 * @brief 调整文件偏移量
 * @param file 文件对象
 * @param offset 偏移量
 * @param whence 基准位置
 * @return Expected<uint64_t> 新的偏移量或错误
 * @pre file != nullptr
 * @note 如果 whence 为 kEnd 且 offset 为正，可能超过文件末尾
 * @note 返回的偏移量是绝对位置（从文件开头计算）
 */
[[nodiscard]] auto Seek(File* file, int64_t offset, SeekWhence whence)
    -> Expected<uint64_t>;

/**
 * @brief 创建目录
 * @param path 目录路径
 * @return Expected<void> 成功或错误
 * @pre path != nullptr
 * @note 父目录必须存在
 * @note 如果目录已存在会返回错误
 */
[[nodiscard]] auto MkDir(const char* path) -> Expected<void>;

/**
 * @brief 删除目录
 * @param path 目录路径
 * @return Expected<void> 成功或错误
 * @pre path != nullptr
 * @note 目录必须为空（不含子项）
 * @note 不能删除挂载点
 */
[[nodiscard]] auto RmDir(const char* path) -> Expected<void>;

/**
 * @brief 删除文件
 * @param path 文件路径
 * @return Expected<void> 成功或错误
 * @pre path != nullptr
 * @note 不能删除目录（使用 RmDir）
 * @note 如果多个硬链接，只会减少链接计数
 */
[[nodiscard]] auto Unlink(const char* path) -> Expected<void>;

/**
 * @brief 读取目录内容
 * @param file 目录文件对象
 * @param dirent 输出目录项数组
 * @param count 最多读取的条目数
 * @return Expected<size_t> 实际读取的条目数或错误
 * @pre file != nullptr && file->inode->type == FileType::kDirectory
 * @note 会返回 . 和 .. 目录项
 * @note 多次调用可遍历整个目录，自动维护偏移量
 */
[[nodiscard]] auto ReadDir(File* file, DirEntry* dirent, size_t count)
    -> Expected<size_t>;

/**
 * @brief 获取根目录 dentry
 * @return Dentry* 根目录 dentry
 * @pre VFS 已初始化且根文件系统已挂载
 * @note 返回 nullptr 表示根文件系统未挂载
 * @note 返回的指针由 VFS 内部管理，不应释放
 */
[[nodiscard]] auto GetRootDentry() -> Dentry*;

}  // namespace vfs

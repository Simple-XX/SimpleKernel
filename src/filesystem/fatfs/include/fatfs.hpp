/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <ff.h>

#include <array>
#include <cstdint>

#include "block_device.hpp"
#include "filesystem.hpp"
#include "vfs.hpp"

namespace fatfs {

/**
 * @brief FatFS VFS 适配器
 * @details 将 FatFS (f_mount / f_open / f_read / ...) 封装在 vfs::FileSystem
 *          接口后。每个 FatFsFileSystem 实例独占一个 FatFS 逻辑驱动器（卷）。
 *
 * @pre FF_VOLUMES >= 1 (ffconf.h)
 */
class FatFsFileSystem : public vfs::FileSystem {
 public:
  /// 根目录权限位
  static constexpr uint32_t kRootDirPermissions = 0755U;
  /// 普通文件默认权限位
  static constexpr uint32_t kDefaultFilePermissions = 0644U;
  /// 路径缓冲区大小（字节）
  static constexpr size_t kPathBufSize = 512;
  /// inode 池容量
  static constexpr size_t kMaxInodes = 256;
  /// 同时打开文件数上限
  static constexpr size_t kMaxOpenFiles = 16;
  /// 同时打开目录数上限
  static constexpr size_t kMaxOpenDirs = 8;

  /**
   * @brief 构造函数，绑定到指定 FatFS 卷号
   * @param volume_id FatFS 逻辑驱动器编号 (0 .. FF_VOLUMES-1)
   * @pre volume_id < FF_VOLUMES
   */
  explicit FatFsFileSystem(uint8_t volume_id);

  /// @name 构造/析构函数
  /// @{
  ~FatFsFileSystem() override;
  FatFsFileSystem(const FatFsFileSystem&) = delete;
  FatFsFileSystem(FatFsFileSystem&&) = delete;
  auto operator=(const FatFsFileSystem&) -> FatFsFileSystem& = delete;
  auto operator=(FatFsFileSystem&&) -> FatFsFileSystem& = delete;
  /// @}

  /**
   * @brief 返回 "fatfs"
   */
  [[nodiscard]] auto GetName() const -> const char* override;

  /**
   * @brief 挂载 FatFS 卷
   * @param device 提供存储的块设备，不能为 nullptr
   * @return Expected<vfs::Inode*> 根目录 inode 或错误
   * @pre device != nullptr
   * @post 返回的 inode->type == vfs::FileType::kDirectory
   */
  auto Mount(vfs::BlockDevice* device) -> Expected<vfs::Inode*> override;

  /**
   * @brief 卸载 FatFS 卷
   * @return Expected<void> 成功或错误
   * @pre 没有引用本卷的打开文件对象
   */
  auto Unmount() -> Expected<void> override;

  /**
   * @brief 刷新所有脏缓冲区
   * @return Expected<void> 成功或错误
   */
  auto Sync() -> Expected<void> override;

  /**
   * @brief 分配新 inode（由 FatFS FILINFO 快照支撑）
   * @return Expected<vfs::Inode*> 新 inode 或错误
   */
  auto AllocateInode() -> Expected<vfs::Inode*> override;

  /**
   * @brief 释放 inode
   * @param inode 要释放的 inode，不能为 nullptr
   * @return Expected<void> 成功或错误
   * @pre inode != nullptr
   * @pre inode->link_count == 0
   */
  auto FreeInode(vfs::Inode* inode) -> Expected<void> override;

  /**
   * @brief 返回本文件系统的 FileOps 实例
   */
  [[nodiscard]] auto GetFileOps() -> vfs::FileOps* override;

  /**
   * @brief 为 inode 打开底层 FatFS FIL 对象
   * @param inode      要打开的 inode（不能为 nullptr，类型必须为 kRegular）
   * @param open_flags vfs OpenFlags 位掩码
   * @return Expected<void> 成功或错误
   * @pre inode != nullptr && inode->type == vfs::FileType::kRegular
   * @post 成功时 inode->fs_private->fil != nullptr
   */
  auto OpenFil(vfs::Inode* inode, vfs::OpenFlags open_flags) -> Expected<void>;

  /**
   * @brief 注册块设备（由 Mount 调用，供 diskio.cpp 的 C 回调使用）
   * @param pdrv     FatFS 物理驱动器号（== volume_id）
   * @param device   块设备指针（nullptr 表示注销）
   */
  static auto SetBlockDevice(uint8_t pdrv, vfs::BlockDevice* device) -> void;

  /**
   * @brief 获取指定驱动器的块设备
   * @param pdrv FatFS 物理驱动器号
   * @return vfs::BlockDevice* 若未注册则返回 nullptr
   */
  [[nodiscard]] static auto GetBlockDevice(uint8_t pdrv) -> vfs::BlockDevice*;

  /**
   * @brief FatFS inode 操作实现
   */
  class FatFsInodeOps : public vfs::InodeOps {
   public:
    explicit FatFsInodeOps(FatFsFileSystem* fs) : fs_(fs) {}

    /**
     * @brief 在目录中查找指定名称的 inode
     * @param dir  目录 inode
     * @param name 文件或子目录名称
     * @return Expected<vfs::Inode*> 找到的 inode 或错误
     */
    auto Lookup(vfs::Inode* dir, const char* name)
        -> Expected<vfs::Inode*> override;

    /**
     * @brief 在目录中创建文件或子目录
     * @param dir  父目录 inode
     * @param name 新条目名称
     * @param type 文件类型（kRegular 或 kDirectory）
     * @return Expected<vfs::Inode*> 新 inode 或错误
     */
    auto Create(vfs::Inode* dir, const char* name, vfs::FileType type)
        -> Expected<vfs::Inode*> override;

    /**
     * @brief 删除目录中的文件条目
     * @param dir  父目录 inode
     * @param name 文件名称
     * @return Expected<void> 成功或错误
     */
    auto Unlink(vfs::Inode* dir, const char* name) -> Expected<void> override;

    /**
     * @brief 在目录中创建子目录
     * @param dir  父目录 inode
     * @param name 子目录名称
     * @return Expected<vfs::Inode*> 新目录 inode 或错误
     */
    auto Mkdir(vfs::Inode* dir, const char* name)
        -> Expected<vfs::Inode*> override;

    /**
     * @brief 删除目录中的空子目录
     * @param dir  父目录 inode
     * @param name 子目录名称
     * @return Expected<void> 成功或错误
     */
    auto Rmdir(vfs::Inode* dir, const char* name) -> Expected<void> override;

   private:
    FatFsFileSystem* fs_;
  };

  /**
   * @brief FatFS 文件操作实现
   */
  class FatFsFileOps : public vfs::FileOps {
   public:
    explicit FatFsFileOps(FatFsFileSystem* fs) : fs_(fs) {}

    /**
     * @brief 打开文件，准备底层 FIL 句柄
     * @param file 文件对象
     * @return Expected<void> 成功或错误
     */
    auto Open(vfs::File* file) -> Expected<void> override;

    /**
     * @brief 从文件读取数据
     * @param file  文件对象
     * @param buf   读取缓冲区
     * @param count 请求读取的字节数
     * @return Expected<size_t> 实际读取字节数或错误
     */
    auto Read(vfs::File* file, void* buf, size_t count)
        -> Expected<size_t> override;

    /**
     * @brief 向文件写入数据
     * @param file  文件对象
     * @param buf   写入数据缓冲区
     * @param count 请求写入的字节数
     * @return Expected<size_t> 实际写入字节数或错误
     */
    auto Write(vfs::File* file, const void* buf, size_t count)
        -> Expected<size_t> override;

    /**
     * @brief 移动文件读写位置
     * @param file   文件对象
     * @param offset 偏移量
     * @param whence 定位基准（kSet / kCur / kEnd）
     * @return Expected<uint64_t> 新的文件偏移量或错误
     */
    auto Seek(vfs::File* file, int64_t offset, vfs::SeekWhence whence)
        -> Expected<uint64_t> override;

    /**
     * @brief 关闭文件，释放底层 FIL 对象
     * @param file 文件对象
     * @return Expected<void> 成功或错误
     */
    auto Close(vfs::File* file) -> Expected<void> override;

    /**
     * @brief 读取目录条目
     * @param file   目录文件对象
     * @param dirent 输出缓冲区
     * @param count  最多读取的条目数
     * @return Expected<size_t> 实际读取条目数或错误
     */
    auto ReadDir(vfs::File* file, vfs::DirEntry* dirent, size_t count)
        -> Expected<size_t> override;

   private:
    FatFsFileSystem* fs_;
  };

  friend class FatFsInodeOps;
  friend class FatFsFileOps;

 private:
  /// FatFS 逻辑驱动器号
  uint8_t volume_id_;
  /// FatFS 文件系统对象（每卷一个）
  FATFS fatfs_obj_;
  /// 根目录 inode（Mount 时设置）
  vfs::Inode* root_inode_ = nullptr;
  /// 当前卷是否已挂载
  bool mounted_{false};

  /// FatFS inode 私有数据
  struct FatInode {
    vfs::Inode inode;
    /// 卷内绝对路径（供 FatFS 操作使用）
    std::array<char, kPathBufSize> path{};
    /// FIL 对象（普通文件打开时使用）；目录或未使用时为 nullptr
    FIL* fil = nullptr;
    /// 该槽位是否在使用
    bool in_use = false;
    /// DIR 对象（目录迭代状态）；未迭代时为 nullptr
    DIR* dir = nullptr;
  };

  std::array<FatInode, kMaxInodes> inodes_;

  /// FIL 对象池
  struct FatFileHandle {
    FIL fil;
    bool in_use = false;
  };

  std::array<FatFileHandle, kMaxOpenFiles> fil_pool_;

  /// DIR 对象池条目
  struct FatDirHandle {
    DIR dir;
    bool in_use = false;
  };

  std::array<FatDirHandle, kMaxOpenDirs> dir_pool_;

  /// inode 操作单例
  FatFsInodeOps inode_ops_;
  /// 文件操作单例
  FatFsFileOps file_ops_;

  /// 每卷块设备注册表（静态，供 diskio.cpp C 回调访问）
  static std::array<vfs::BlockDevice*, FF_VOLUMES> block_devices_;

  /**
   * @brief 从 inode 池中分配一个空闲槽位
   * @return FatInode* 成功返回指针，池满返回 nullptr
   */
  [[nodiscard]] auto AllocateFatInode() -> FatInode*;

  /**
   * @brief 释放 inode 池槽位
   * @param fi 要释放的槽位，允许为 nullptr（此时无操作）
   */
  auto FreeFatInode(FatInode* fi) -> void;

  /**
   * @brief 从 FIL 池中分配一个空闲 FIL 对象
   * @return FIL* 成功返回指针，池满返回 nullptr
   */
  [[nodiscard]] auto AllocateFil() -> FIL*;

  /**
   * @brief 归还 FIL 对象到池中
   * @param fil 要归还的 FIL 指针
   */
  auto FreeFil(FIL* fil) -> void;

  /**
   * @brief 从 DIR 池中分配一个空闲 DIR 对象
   * @return DIR* 成功返回指针，池满返回 nullptr
   */
  [[nodiscard]] auto AllocateDir() -> DIR*;

  /**
   * @brief 归还 DIR 对象到池中
   * @param dir 要归还的 DIR 指针
   */
  auto FreeDir(DIR* dir) -> void;
};

}  // namespace fatfs

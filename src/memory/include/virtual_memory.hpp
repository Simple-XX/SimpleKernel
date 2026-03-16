/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>
#include <etl/singleton.h>

#include <cstddef>
#include <cstdint>

#include "expected.hpp"

/**
 * @brief 虚拟内存管理器（具体类，非抽象基类）
 *
 * @details 本类是内核虚拟内存子系统的唯一实现，通过
 *          cpu_io::virtual_memory 命名空间中的架构相关辅助函数
 *          实现跨架构（x86_64/riscv64/aarch64）的页表操作。
 *          架构差异由 cpu_io 库在编译期分发，本类本身不使用虚函数
 *          或条件编译。
 *
 *          物理页帧的分配/释放由外部分配器（bmalloc）提供，
 *          本类通过 aligned_alloc/aligned_free 间接调用。
 *
 * @note 地址操作、页表项操作等实用函数请使用 cpu_io 库提供的接口
 * @note 以 VirtualMemorySingleton 全局单例方式使用
 */
class VirtualMemory {
 public:
  /**
   * @brief 初始化当前核心的页表
   * @pre 内核页表目录已初始化
   * @post 当前核心的页表目录已设置并启用分页
   */
  auto InitCurrentCore() const -> void;

  /**
   * @brief 映射设备内存 (MMIO)
   * @param phys_addr 设备物理基地址
   * @param size 映射大小
   * @param flags 页表属性，默认为内核设备内存属性（如果架构支持区分的话）
   * @return Expected<void*> 映射后的虚拟地址，失败时返回错误
   */
  [[nodiscard]] auto MapMMIO(
      uint64_t phys_addr, size_t size,
      uint32_t flags = cpu_io::virtual_memory::GetKernelPagePermissions())
      -> Expected<void*>;

  /**
   * @brief 映射单个页面
   * @param page_dir 页表目录
   * @param virtual_addr 虚拟地址
   * @param physical_addr 物理地址
   * @param flags 页表属性
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] auto MapPage(void* page_dir, void* virtual_addr,
                             void* physical_addr, uint32_t flags)
      -> Expected<void>;

  /**
   * @brief 取消映射单个页面
   * @param page_dir 页表目录
   * @param virtual_addr 虚拟地址
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  [[nodiscard]] auto UnmapPage(void* page_dir, void* virtual_addr)
      -> Expected<void>;

  /**
   * @brief 获取虚拟地址对应的物理地址映射
   * @param page_dir 页表目录
   * @param virtual_addr 虚拟地址
   * @return Expected<void*> 物理地址，失败时返回错误
   */
  [[nodiscard]] auto GetMapping(void* page_dir, void* virtual_addr)
      -> Expected<void*>;

  /**
   * @brief 回收页表，释放所有映射和子页表
   * @param page_dir 要回收的页表目录
   * @param free_pages 是否同时释放映射的物理页
   * @note 此函数会递归释放所有层级的页表
   */
  auto DestroyPageDirectory(void* page_dir, bool free_pages = false) -> void;

  /**
   * @brief 复制页表
   * @param src_page_dir 源页表目录
   * @param copy_mappings 是否复制映射（true：复制映射，false：仅复制页表结构）
   * @return Expected<void*> 新页表目录，失败时返回错误
   * @note 如果 copy_mappings 为 true，会复制所有的页表项；
   *       如果为 false，只复制页表结构，不复制最后一级的映射
   */
  [[nodiscard]] auto ClonePageDirectory(void* src_page_dir,
                                        bool copy_mappings = true)
      -> Expected<void*>;

  /// @name 构造/析构函数
  /// @{
  /**
   * @brief 构造函数
   * @post 内核页表目录已分配并清零
   * @post 全部物理内存已映射
   */
  VirtualMemory();
  VirtualMemory(const VirtualMemory&) = delete;
  VirtualMemory(VirtualMemory&&) = default;
  auto operator=(const VirtualMemory&) -> VirtualMemory& = delete;
  auto operator=(VirtualMemory&&) -> VirtualMemory& = default;
  ~VirtualMemory() = default;
  /// @}

 private:
  void* kernel_page_dir_{nullptr};

  static constexpr size_t kEntriesPerTable =
      cpu_io::virtual_memory::kPageSize / sizeof(void*);

  /**
   * @brief 递归释放页表
   * @param table 当前页表
   * @param level 当前层级
   * @param free_pages 是否释放物理页
   */
  auto RecursiveFreePageTable(uint64_t* table, size_t level, bool free_pages)
      -> void;
  /**
   * @brief 递归复制页表
   * @param src_table 源页表
   * @param dst_table 目标页表
   * @param level 当前层级
   * @param copy_mappings 是否复制映射
   * @return Expected<void> 成功时返回 void，失败时返回错误
   */
  auto RecursiveClonePageTable(uint64_t* src_table, uint64_t* dst_table,
                               size_t level, bool copy_mappings)
      -> Expected<void>;

  /**
   * @brief 在页表中查找虚拟地址对应的页表项
   * @param page_dir         页目录
   * @param virtual_addr     虚拟地址
   * @param allocate         如果页表项不存在是否分配新的页表
   * @return Expected<uint64_t*> 页表项指针，失败时返回错误
   */
  [[nodiscard]] auto FindPageTableEntry(void* page_dir, void* virtual_addr,
                                        bool allocate = false)
      -> Expected<uint64_t*>;
};

using VirtualMemorySingleton = etl::singleton<VirtualMemory>;

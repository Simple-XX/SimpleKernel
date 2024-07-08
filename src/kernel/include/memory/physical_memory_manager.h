
/**
 * @file physical_memory_manager.h
 * @brief 物理内存管理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_PHYSICAL_MEMORY_MANAGER_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_PHYSICAL_MEMORY_MANAGER_H_

#include <cstddef>
#include <cstdint>

#include "memory/allocator_base.hpp"
#include "singleton.hpp"

/**
 * @brief 物理内存管理接口
 * 对物理内存的管理来说
 * 1. 管理所有的物理内存，不论是否被机器保留/无法访问
 * 2. 内存开始地址与长度由 bootloader 给出: x86 下为 grub, riscv 下为 opensbi
 * 3.
 *    不关心内存是否被使用，但是默认的物理内存分配空间从内核结束后开始
 *    如果由体系结构需要分配内核开始前内存空间的，则尽量避免
 * 4. 最管理单位为页
 */
class PhysicalMemoryManager {
 public:
  /**
   * @brief 构造函数
   * @param addr 物理地址起点
   * @param pages_count 物理页数
   */
  explicit PhysicalMemoryManager(uint64_t addr, size_t pages_count);

  /// @name 构造/析构函数
  /// @{
  PhysicalMemoryManager() = default;
  PhysicalMemoryManager(const PhysicalMemoryManager &) = default;
  PhysicalMemoryManager(PhysicalMemoryManager &&) = default;
  auto operator=(const PhysicalMemoryManager &) -> PhysicalMemoryManager & =
                                                       default;
  auto operator=(PhysicalMemoryManager &&) -> PhysicalMemoryManager & = default;
  ~PhysicalMemoryManager() = default;
  /// @}

  /**
   * @brief 获取物理内存页数
   * @return size_t 物理内存页数
   */
  size_t GetPagesCount() const;

  /**
   * @brief 获取内核空间起始地址
   * @return uint64_t        内核空间起始地址
   */
  uint64_t GetKernelSpaceAddr() const;

  /**
   * @brief 获取内核空间页数
   * @return size_t 内核空间页数
   */
  size_t GetKernelSpacePagesCount() const;

  /**
   * @brief 获取用户间起始地址
   * @return uint64_t 用户间起始地址
   */
  uint64_t GetUserSpaceAddr() const;

  /**
   * @brief 获取用户间页数
   * @return size_t 用户间页数
   */
  size_t GetUserSpacePagesCount() const;

  /**
   * @brief 获取当前已使用页数
   * @return size_t 已使用页数
   */
  size_t GetUsedPagesCount() const;

  /**
   * @brief 获取当前空闲页数
   * @return size_t 空闲页数
   */
  size_t GetFreePagesCount() const;

  /**
   * @brief 分配一页
   * @return uint64_t       分配的内存起始地址
   */
  uint64_t AllocUserPage();

  /**
   * @brief 分配多页
   * @param  _len            页数
   * @return uint64_t       分配的内存起始地址
   */
  uint64_t AllocUserPages(size_t _len);

  /**
   * @brief 分配以指定地址开始的 _len 页
   * @param  addr           指定的地址
   * @param  _len            页数
   * @return true            成功
   * @return false           失败
   */
  bool AllocUserPagesAt(uint64_t addr, size_t _len);

  /**
   * @brief 在内核空间申请一页
   * @return uint64_t       分配的内存起始地址
   */
  uint64_t AllocKernelPage();

  /**
   * @brief 在内核空间分配 pages_count 页
   * @param  pages_count 页数
   * @return uint64_t 分配到的内存起始地址
   */
  uint64_t AllocKernelPages(size_t pages_count);

  /**
   * @brief 在内核空间分配以指定地址开始的 _len 页
   * @param addr 指定的地址
   * @param pages_count 页数
   * @return true 成功
   * @return false 失败
   */
  bool AllocKernelPagesAt(uint64_t addr, size_t pages_count);

  /**
   * @brief 回收一页
   * @param addr 要回收的地址
   */
  void FreePage(uint64_t addr);

  /**
   * @brief 回收多页
   * @param addr 要回收的地址
   * @param pages_count 页数
   */
  void FreePages(uint64_t addr, size_t pages_count);

 private:
  /// 物理内存开始地址
  uint64_t addr_;
  /// 物理内存页数
  size_t pages_count_;
  /// 内核空间起始地址
  uint64_t kernel_addr_;
  /// 内核页数
  size_t kernel_pages_count_;
  /// 用户空间起始地址
  uint64_t user_start_;
  /// 用户空间页数
  size_t user_pages_count_;

  /// 内核空间不会位于内存中间，导致出现用户间被切割为两部分的情况
  /// 物理内存分配器，分配内核空间
  AllocatorBase *kernel_allocator_;
  /// 物理内存分配器，分配用户空间
  AllocatorBase *user_allocator_;

  /**
   * @brief 将 elf 与 dtb
   * 信息移动到内核空间，位于内核结束后的下一页，分别占用一页
   */
  void MoveElfDtb();
};

/// 全局物理内存管理器
[[maybe_unused]] static Singleton<PhysicalMemoryManager> kPhysicalMemoryManager;

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_PHYSICAL_MEMORY_MANAGER_H_ */

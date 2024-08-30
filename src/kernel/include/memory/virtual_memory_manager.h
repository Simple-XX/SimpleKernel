
/**
 * @file virtual_memory_manager.h
 * @brief 虚拟内存管理
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_VIRTUAL_MEMORY_MANAGER_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_VIRTUAL_MEMORY_MANAGER_H_

#include <cstddef>
#include <cstdint>

#include "cpu.hpp"
#include "physical_memory_manager.h"
#include "singleton.hpp"

// TODO: 可以优化

/// 页表项，最底层
typedef uintptr_t pte_t;
/// 页表，也可以是页目录，它们的结构是一样的
typedef uintptr_t* pt_t;

/// 每个页表能映射多少页 = 页大小/页表项大小: 2^9
static constexpr const size_t VMM_PAGES_PRE_PAGE_TABLE =
    PhysicalMemoryManager::kPageSize / sizeof(pte_t);

/// 映射内核空间的大小
static constexpr const size_t VMM_KERNEL_SPACE_SIZE = COMMON::KERNEL_SPACE_SIZE;

/// 内核映射的页数
static constexpr const size_t VMM_KERNEL_SPACE_PAGES =
    VMM_KERNEL_SPACE_SIZE / PhysicalMemoryManager::kPageSize;

/**
 * @brief 虚拟内存抽象
 * 设计目标：在内核中存在若干个 VirtualMemoryManager
 * 实例，内核拥有一个静态实例，每个进程有独立的 VirtualMemoryManager
 * 另外要考虑：每个 cpu 维护自己的页寄存器，调度算法需要考虑到
 * - 页表（保存到寄存器）
 *  - 一级页表项
 *  - 二级页表项
 *  - 三级页表项（最低级）
 */
class VirtualMemoryManager {
 private:
  /**
   * @brief 物理地址转换到页表项
   * @param  _pa             物理地址
   * @return constexpr uintptr_t 对应的虚拟地址
   * @note 0~11: pte 属性
   * 12~31: 页表的物理页地址
   */
  static constexpr uintptr_t PA2PTE(uintptr_t _pa) {
    return (_pa >> cpu::vmm_info::VMM_PAGE_OFF_BITS)
           << cpu::vmm_info::VMM_PTE_PROP_BITS;
  }

  /**
   * @brief 页表项转换到物理地址
   * @param  _pte            页表
   * @return constexpr uintptr_t 对应的物理地址
   */
  static constexpr uintptr_t PTE2PA(const pte_t _pte) {
    return (((uintptr_t)_pte) >> cpu::vmm_info::VMM_PTE_PROP_BITS)
           << cpu::vmm_info::VMM_PAGE_OFF_BITS;
  }

  /**
   * @brief 计算 X 级页表的位置
   * @param  _level          级别
   * @return constexpr uintptr_t 偏移
   */
  static constexpr uintptr_t PXSHIFT(const size_t _level) {
    return cpu::vmm_info::VMM_PAGE_OFF_BITS +
           (cpu::vmm_info::VMM_VPN_BITS * _level);
  }

  /**
   * @brief 获取 _va 的第 _level 级 VPN
   * @note 例如虚拟地址右移 12+(10 * _level) 位，
   * 得到的就是第 _level 级页表的 VPN
   */
  static constexpr uintptr_t PX(size_t _level, uintptr_t _va) {
    return (_va >> PXSHIFT(_level)) & cpu::vmm_info::VMM_VPN_BITS_MASK;
  }

  /**
   * @brief 在 _pgd 中查找 _va 对应的页表项
   * 如果未找到，_alloc 为真时会进行分配
   * @param  _pgd            要查找的页目录
   * @param  _va             虚拟地址
   * @param  _alloc          是否分配
   * @return pte_t*          未找到返回 nullptr
   */
  pte_t* find(const pt_t _pgd, uintptr_t _va, bool _alloc);

 protected:
 public:
  /**
   * @brief 获取单例
   * @return VirtualMemoryManager&             静态对象
   */
  static VirtualMemoryManager& get_instance(void);

  /**
   * @brief 初始化
   * @return true            成功
   * @return false           失败
   */
  bool init(void);

  /**
   * @brief 获取当前页目录
   * @return pt_t            当前页目录
   */
  pt_t get_pgd(void);

  /**
   * @brief 设置当前页目录
   * @param  _pgd            要设置的页目录
   */
  void set_pgd(const pt_t _pgd);

  /**
   * @brief 映射物理地址到虚拟地址
   * @param  _pgd            要使用的页目录
   * @param  _va             要映射的虚拟地址
   * @param  _pa             物理地址
   * @param  _flag           属性
   */
  void mmap(const pt_t _pgd, uintptr_t _va, uintptr_t _pa, uint32_t _flag);

  /**
   * @brief 取消映射
   * @param  _pgd            要操作的页目录
   * @param  _va             要取消映射的虚拟地址
   */
  void unmmap(const pt_t _pgd, uintptr_t _va);

  /**
   * @brief 获取映射的物理地址
   * @param  _pgd            页目录
   * @param  _va             虚拟地址
   * @param  _pa             如果已经映射，保存映射的物理地址，否则为 nullptr
   * @return true            已映射
   * @return false           未映射
   */
  bool get_mmap(const pt_t _pgd, uintptr_t _va, const void* _pa);
};

/// 全局虚拟内存管理器
[[maybe_unused]] static Singleton<VirtualMemoryManager> kVirtualMemoryManager;

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_VIRTUAL_MEMORY_MANAGER_H_ */

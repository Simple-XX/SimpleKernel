
/**
 * @file virtual_memory_manager.cpp
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

#include "memory/virtual_memory_manager.h"

#include <functional>

#include "basic_info.hpp"
#include "kernel_log.hpp"
#include "sk_cstdio"
#include "sk_cstring"

VirtualMemoryManager::VirtualMemoryManager(uint32_t, uint8_t*) {
  auto kernel_pgd = cpu::vmm::GetPageDirectory();
  uint64_t pa = 0;

  auto res =
      get_mmap((pt_t)kernel_pgd, kBasicInfo.GetInstance().kernel_addr, &pa);
  klog::Debug("res: %d, pa 0x%X\n", res, pa);

  // 分配一页用于保存页目录
  // pt_t pgd_kernel = (pt_t)AllocKernelPage();
  // memset(pgd_kernel, 0, PhysicalMemoryManager::kPageSize);
  // // 映射内核空间
  // for (uint64_t addr = kBasicInfo.GetInstance().kernel_addr;
  //      addr < kBasicInfo.GetInstance().kernel_addr + kKernelSpaceSize;
  //      addr += PhysicalMemoryManager::kPageSize) {
  //   // TODO: 区分代码/数据等段分别映射
  //   mmap(pgd_kernel, addr, addr,
  //        cpu::vmm::VMM_PAGE_READABLE | cpu::vmm::VMM_PAGE_WRITABLE |
  //            cpu::vmm::VMM_PAGE_EXECUTABLE);
  // }
  // 设置页目录
  // cpu::vmm::DisablePage();
  // klog::Debug("set_pgd: 0x%X\n", pgd_kernel);
  // set_pgd(pgd_kernel);
  // 开启分页
  // cpu::vmm::EnablePage();
}

pt_t VirtualMemoryManager::get_pgd(void) {
  return (pt_t)cpu::vmm::GetPageDirectory();
}

void VirtualMemoryManager::set_pgd(const pt_t _pgd) {
  // 设置页目录
  cpu::vmm::SetPageDirectory((uint64_t)_pgd);
  // 刷新缓存
  cpu::vmm::FlushPage(0);
  return;
}

void VirtualMemoryManager::mmap(const pt_t _pgd, uintptr_t _va, uintptr_t _pa,
                                uint32_t _flag) {
  pte_t* pte = find(_pgd, _va, true);
  // 一般情况下不应该为空
  if (pte == nullptr) {
    klog::Err("pte == nullptr\n");
    throw;
  }

  // 已经映射过了 且 flag 没有变化
  if (((*pte & cpu::vmm::VMM_PAGE_VALID) == cpu::vmm::VMM_PAGE_VALID) &&
      ((*pte & ((1 << cpu::vmm::VMM_PTE_PROP_BITS) - 1)) == _flag)) {
    klog::Warn("remap.\n");
  }
  // 没有映射，或更改了 flag
  else {
    // 那么设置 *pte
    // pte 解引用后的值是页表项
    *pte = PA2PTE(_pa) | _flag |
           (*pte & ((1 << cpu::vmm::VMM_PTE_PROP_BITS) - 1)) |
           cpu::vmm::VMM_PAGE_VALID;
    // 刷新缓存
    cpu::vmm::FlushPage(0);
  }
  return;
}

void VirtualMemoryManager::unmmap(const pt_t _pgd, uintptr_t _va) {
  pte_t* pte = find(_pgd, _va, false);
  // 找到页表项
  // 未找到
  if (pte == nullptr) {
    klog::Warn("VirtualMemoryManager::unmmap: find.\n");
    return;
  }
  // 找到了，但是并没有被映射
  if ((*pte & cpu::vmm::VMM_PAGE_VALID) == 0) {
    klog::Warn("VirtualMemoryManager::unmmap: not mapped.\n");
  }
  // 置零
  *pte = 0x00;
  // 刷新缓存
  cpu::vmm::FlushPage(0);
  // TODO: 如果一页表都被 unmap，释放占用的物理内存
  return;
}

bool VirtualMemoryManager::get_mmap(const pt_t _pgd, uintptr_t _va,
                                    const void* _pa) {
  pte_t* pte = find(_pgd, _va, false);
  bool res = false;
  // pte 不为空且有效，说明映射了
  if ((pte != nullptr) && ((*pte & cpu::vmm::VMM_PAGE_VALID) == 1)) {
    // 如果 _pa 不为空
    if (_pa != nullptr) {
      // 设置 _pa
      // 将页表项转换为物理地址
      *(uintptr_t*)_pa = PTE2PA(*pte);
    }
    // 返回 true
    res = true;
  }
  // 否则说明没有映射
  else {
    // 如果 _pa 不为空
    if (_pa != nullptr) {
      // 设置 _pa
      *(uintptr_t*)_pa = (uintptr_t) nullptr;
    }
  }
  return res;
}

// 在 _pgd 中查找 _va 对应的页表项
// 如果未找到，_alloc 为真时会进行分配
pte_t* VirtualMemoryManager::find(const pt_t _pgd, uintptr_t _va, bool _alloc) {
  pt_t pgd = _pgd;
  // sv39 共有三级页表，一级一级查找
  // -1 是因为最后一级是具体的某一页，在函数最后直接返回
  for (size_t level = cpu::vmm::VMM_PT_LEVEL - 1; level > 0; level--) {
    // 每次循环会找到 _va 的第 level 级页表 pgd
    // 相当于 pgd_level[VPN_level]，这样相当于得到了第 level 级页表的地址
    pte_t* pte = (pte_t*)&pgd[PX(level, _va)];
    // 解引用 pte，如果有效，获取 level+1 级页表，
    if ((*pte & cpu::vmm::VMM_PAGE_VALID) == 1) {
      // pgd 指向下一级页表
      // *pte 保存的是页表项，需要转换为对应的物理地址
      pgd = (pt_t)PTE2PA(*pte);
    }
    // 如果无效
    else {
      // 判断是否需要分配
      // 如果需要
      if (_alloc == true) {
        // 申请新的物理页
        pgd = (pt_t)AllocKernelPage();
        memset(pgd, 0, PhysicalMemoryManager::kPageSize);
        // 申请失败则返回
        if (pgd == nullptr) {
          // 如果出现这种情况，说明物理内存不够，一般不会出现
          klog::Err("No Enough Memory\n");
          throw;
          return nullptr;
        }
        // 清零
        memset(pgd, 0, PhysicalMemoryManager::kPageSize);
        // 填充页表项
        *pte = PA2PTE((uintptr_t)pgd) | cpu::vmm::VMM_PAGE_VALID;
      }
      // 不分配的话直接返回
      else {
        return nullptr;
      }
    }
  }
  // 0 最低级 pt
  return &pgd[PX(0, _va)];
}

uint32_t VirtualMemoryInit(uint32_t argc, uint8_t* argv) {
  // 初始化虚拟内存管理器
  kVirtualMemoryManager.GetInstance() = VirtualMemoryManager(argc, argv);

  klog::Info("Hello VirtualMemoryInit\n");

  return 0;
}

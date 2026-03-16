/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "task_control_block.hpp"

#include <cpu_io.h>
#include <elf.h>

#include "arch.h"
#include "basic_info.hpp"
#include "interrupt_base.h"
#include "kernel.h"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "sk_stdlib.h"
#include "virtual_memory.hpp"

namespace {

auto LoadElf(const uint8_t* elf_data, uint64_t* page_table) -> uint64_t {
  // Check ELF magic
  auto* ehdr = reinterpret_cast<const Elf64_Ehdr*>(elf_data);
  if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
    klog::Err("Invalid ELF magic");
    return 0;
  }

  auto* phdr = reinterpret_cast<const Elf64_Phdr*>(elf_data + ehdr->e_phoff);
  auto& vm = VirtualMemorySingleton::instance();

  for (int i = 0; i < ehdr->e_phnum; ++i) {
    if (phdr[i].p_type != PT_LOAD) continue;

    uintptr_t vaddr = phdr[i].p_vaddr;
    uintptr_t memsz = phdr[i].p_memsz;
    uintptr_t filesz = phdr[i].p_filesz;
    uintptr_t offset = phdr[i].p_offset;

    uint32_t flags = cpu_io::virtual_memory::GetUserPagePermissions(
        (phdr[i].p_flags & PF_R) != 0, (phdr[i].p_flags & PF_W) != 0,
        (phdr[i].p_flags & PF_X) != 0);

    uintptr_t start_page = cpu_io::virtual_memory::PageAlign(vaddr);
    uintptr_t end_page = cpu_io::virtual_memory::PageAlignUp(vaddr + memsz);

    for (uintptr_t page = start_page; page < end_page;
         page += cpu_io::virtual_memory::kPageSize) {
      void* p_page = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                   cpu_io::virtual_memory::kPageSize);
      if (!p_page) {
        klog::Err("Failed to allocate page for ELF");
        return 0;
      }
      kstd::memset(p_page, 0, cpu_io::virtual_memory::kPageSize);

      // Mapping logic
      uintptr_t v_start = page;
      uintptr_t v_end = page + cpu_io::virtual_memory::kPageSize;

      // Map intersection with file data
      uintptr_t file_start = vaddr;
      uintptr_t file_end = vaddr + filesz;

      uintptr_t copy_start = std::max(v_start, file_start);
      uintptr_t copy_end = std::min(v_end, file_end);

      if (copy_end > copy_start) {
        uintptr_t dst_off = copy_start - v_start;
        uintptr_t src_off = (copy_start - vaddr) + offset;
        kstd::memcpy(static_cast<uint8_t*>(p_page) + dst_off,
                     elf_data + src_off, copy_end - copy_start);
      }

      if (!vm.MapPage(page_table, reinterpret_cast<void*>(page), p_page,
                      flags)) {
        klog::Err("MapPage failed");
        return 0;
      }
    }
  }
  return ehdr->e_entry;
}

}  // namespace

auto TaskControlBlock::GetStatus() const -> etl::fsm_state_id_t {
  return fsm.GetStateId();
}

auto TaskControlBlock::JoinThreadGroup(TaskControlBlock* leader) -> void {
  if (!leader || leader == this) {
    return;
  }

  // 设置 tgid
  aux->tgid = leader->aux->tgid;

  // 在 leader 之后插入自身
  etl::link_splice<ThreadGroupLink>(*leader, *this);
}

auto TaskControlBlock::LeaveThreadGroup() -> void { ThreadGroupLink::unlink(); }

auto TaskControlBlock::GetThreadGroupSize() const -> size_t {
  if (aux->tgid == 0) {
    // 未加入任何线程组
    return 1;
  }

  size_t count = 1;

  // 向前遍历至链表头
  const ThreadGroupLink* curr = etl_previous;
  while (curr) {
    ++count;
    curr = curr->etl_previous;
  }

  // 向后遍历至链表尾
  curr = etl_next;
  while (curr) {
    ++count;
    curr = curr->etl_next;
  }

  return count;
}

TaskControlBlock::TaskControlBlock(const char* _name, int priority,
                                   ThreadEntry entry, void* arg)
    : name(_name) {
  // 分配辅助数据
  aux = new TaskAuxData{};
  if (!aux) {
    klog::Err("Failed to allocate TaskAuxData for task {}", name);
    return;
  }

  // 设置优先级
  sched_info.priority = priority;
  sched_info.base_priority = priority;

  // 分配内核栈
  kernel_stack = static_cast<uint8_t*>(aligned_alloc(
      cpu_io::virtual_memory::kPageSize, kDefaultKernelStackSize));
  if (!kernel_stack) {
    klog::Err("Failed to allocate kernel stack for task {}", name);
    return;
  }

  // 设置 trap_context_ptr 指向内核栈顶预留的位置
  trap_context_ptr = reinterpret_cast<cpu_io::TrapContext*>(
      kernel_stack + kDefaultKernelStackSize - sizeof(cpu_io::TrapContext));

  // 设置内核栈顶
  auto stack_top =
      reinterpret_cast<uint64_t>(kernel_stack) + kDefaultKernelStackSize;

  // 初始化任务上下文
  InitTaskContext(&task_context, entry, arg, stack_top);

  fsm.Start();
}

TaskControlBlock::TaskControlBlock(const char* _name, int priority,
                                   uint8_t* elf, int argc, char** argv)
    : name(_name) {
  // 分配辅助数据
  aux = new TaskAuxData{};
  if (!aux) {
    klog::Err("Failed to allocate TaskAuxData for task {}", name);
    return;
  }

  // 设置优先级
  sched_info.priority = priority;
  sched_info.base_priority = priority;

  /// @todo
  (void)_name;
  (void)priority;
  (void)elf;
  (void)argc;
  (void)argv;
  LoadElf(nullptr, nullptr);

  fsm.Start();
}

TaskControlBlock::~TaskControlBlock() {
  // 从线程组中移除
  LeaveThreadGroup();

  // 释放内核栈
  if (kernel_stack) {
    aligned_free(kernel_stack);
    kernel_stack = nullptr;
  }

  // 释放页表（如果有用户空间页表）
  if (page_table) {
    // 如果是私有页表（非共享），需要释放物理页
    auto should_free_pages = !(aux->clone_flags & clone_flag::kVm);
    VirtualMemorySingleton::instance().DestroyPageDirectory(page_table,
                                                            should_free_pages);
    page_table = nullptr;
  }

  // 释放辅助数据
  if (aux) {
    delete aux;
    aux = nullptr;
  }
}

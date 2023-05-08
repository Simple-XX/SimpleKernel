
/**
 * @file page_fault.cpp
 * @brief 页错误处理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-03-27
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-03-27<td>Zone.Niuzh<td>新建文件
 * </table>
 */

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "vmm.h"
#include "pmm.h"

int32_t pg_load_excp(int, char **) {
    uintptr_t addr = CPU::READ_STVAL();
    uintptr_t pa   = 0x0;
    auto      is_mmap =
        VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), addr, &pa);
    // 如果 is_mmap 为 true，说明已经应映射过了
    if (is_mmap == true) {
        // 直接映射
        VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, pa,
                                 VMM_PAGE_READABLE);
    }
    else {
        // 分配一页物理内存进行映射
        pa = PMM::get_instance().alloc_page_kernel();
        VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, pa,
                                 VMM_PAGE_READABLE);
    }
    info("pg_load_excp done: 0x%p.\n", addr);
    return 0;
}

int32_t pg_store_excp(int, char **) {
    uintptr_t addr = CPU::READ_STVAL();
    uintptr_t pa   = 0x0;
    auto      is_mmap =
        VMM::get_instance().get_mmap(VMM::get_instance().get_pgd(), addr, &pa);
    // 如果 is_mmap 为 true，说明已经应映射过了
    if (is_mmap == true) {
        // 直接映射
        VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, pa,
                                 VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    }
    else {
        // 分配一页物理内存进行映射
        pa = PMM::get_instance().alloc_page_kernel();
        VMM::get_instance().mmap(VMM::get_instance().get_pgd(), addr, pa,
                                 VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    }
    info("pg_store_excp done: 0x%p.\n", addr);
    return 0;
}

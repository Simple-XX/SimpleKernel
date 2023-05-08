
/**
 * @file clint.cpp
 * @brief clint 抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "cpu.hpp"
#include "stdio.h"
#include "vmm.h"
#include "boot_info.h"
#include "resource.h"
#include "intr.h"

CLINT &CLINT::get_instance(void) {
    /// 定义全局 CLINT 对象
    static CLINT clint;
    return clint;
}

int32_t CLINT::init(void) {
    // 映射 clint 地址
    resource_t resource = BOOT_INFO::get_clint();
    for (uintptr_t a = resource.mem.addr;
         a < resource.mem.addr + resource.mem.len; a += COMMON::PAGE_SIZE) {
        VMM::get_instance().mmap(VMM::get_instance().get_pgd(), a, a,
                                 VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    }
    // 开启内部中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_SSIE);
    info("clint init.\n");
    return 0;
}

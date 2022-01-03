
/**
 * @file task.cpp
 * @brief 任务抽象实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "stdlib.h"
#include "stdint.h"
#include "iostream"
#include "task.h"
#include "cpu.hpp"
#include "common.h"
#include "pmm.h"
#include "scheduler.h"
#include "boot_info.h"

task_t::task_t(mystl::string _name, void (*_task)(void))
    : name(_name), spinlock(_name) {
    pid        = -1;
    state      = SLEEPING;
    parent     = nullptr;
    stack      = PMM::get_instance().alloc_pages_kernel(COMMON::STACK_SIZE /
                                                        COMMON::PAGE_SIZE);
    context.ra = (uintptr_t)_task;
    context.callee_regs.sp = stack + COMMON::STACK_SIZE;
    context.sscratch       = (uintptr_t)malloc(sizeof(CPU::context_t));
    page_dir               = VMM::get_instance().get_pgd();
    slice                  = 0;
    slice_total            = 0;
    hartid                 = COMMON::get_curr_core_id(CPU::READ_SP());
    exit_code              = -1;
    return;
}

task_t::~task_t(void) {
    // 停止子进程
    // 回收页目录
    // 回收栈
    PMM::get_instance().free_pages(stack, 4);
    // 回收上下文
    free((void *)context.sscratch);
    // 回收其它
    std::cout << "del task: " << name << std::endl;
    return;
}

std::ostream &operator<<(std::ostream &_os, const task_t &_task) {
    _os << _task.name << ": ";
    printf("pid 0x%X, state 0x%X\n", _task.pid, _task.state);
    std::cout << _task.context;
    return _os;
}

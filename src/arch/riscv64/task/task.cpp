
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

    // 设置进程基本信息
task_t::task_t(mystl::string _name, void (*_task)(void))
    : name(_name), spinlock(_name) {
    pid            = -1;
    // 睡眠状态，等待唤醒
    state          = SLEEPING;
    // @todo 父进程暂时置空
    parent         = nullptr;
    // 从内核地址中分配线程栈
    stack          = PMM::get_instance().alloc_pages_kernel(COMMON::STACK_SIZE /
                                                            COMMON::PAGE_SIZE);
    // ra 地址设为要执行的任务地址，这样在 ret 后会跳转到任务执行
    context.ra     = (uintptr_t)_task;
    // 读取当前 core
    context.coreid = CPU::get_curr_core_id();
    // 设置栈地址
    context.callee_regs.sp = stack + COMMON::STACK_SIZE;
    // 开启分页
    // @todo 设为与当前状态一致
    context.satp.ppn =
        (uint64_t)(VMM::get_instance().get_pgd()) >> CPU::satp_t::PPN_OFFSET;
    context.satp.mode = CPU::satp_t::SV39;
    // 分配用于保存上下文的空间
    context.sscratch  = (uintptr_t)kmalloc(sizeof(CPU::context_t));
    // 打开时钟中断
    context.sie |= CPU::SIE_STIE;
    // 打开 S 态中断
//    context.sstatus.sie = true;
    context.sstatus.val=0x8000000200006022;
    std::cout<<context.sstatus<<std::endl;
    // 设置线程页目录
    // @todo 每个线程独立
    page_dir            = VMM::get_instance().get_pgd();
    // 运行时间设为 0
    slice               = 0;
    slice_total         = 0;
    // 设置当前 core
    hartid              = CPU::get_curr_core_id();
    exit_code           = -1;
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
    printf("%s: hartid: 0x%X, pid 0x%X, state 0x%X, parent: 0x%X, stack: 0x%X",
           _task.name.c_str(), _task.hartid, _task.pid, _task.state,
           _task.parent, _task.stack);
    return _os;
}

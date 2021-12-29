
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// task.cpp for Simple-XX/SimpleKernel.

#include "stdlib.h"
#include "stdint.h"
#include "iostream"
#include "task.h"
#include "cpu.hpp"
#include "common.h"
#include "pmm.h"
#include "scheduler.h"
#include "boot_info.h"

extern "C" void switch_context_init(CPU::context_t *_context);

// 当前任务
TASK::task_t *TASK::curr_task[CPU::CPUS];
// 任务向量
mystl::queue<TASK::task_t *> *TASK::task_queue;
TASK::task_t *                TASK::task_os;

TASK::task_t::task_t(mystl::string _name, void (*_task)(void)) : name(_name) {
    pid      = alloc_pid();
    state    = RUNNING;
    parent   = nullptr;
    page_dir = VMM::get_instance().get_pgd();
    // 栈空间分配在内核空间
    stack                  = PMM::get_instance().alloc_pages_kernel(4);
    context.ra             = (uintptr_t)_task;
    context.callee_regs.sp = stack + COMMON::PAGE_SIZE * 4;
    context.sscratch       = (uintptr_t)malloc(sizeof(CPU::context_t));
    slice                  = 0;
    slice_total            = 0;
    // 将新进程添加到链表
    task_queue->push(this);
    return;
}

TASK::task_t::~task_t(void) {
    // 回收 pid
    free_pid(pid);
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

void TASK::exit(uint32_t _exit_code) {
    // 从队列中删除
    TASK::curr_task[CPU::get_curr_core_id()]->state = ZOMBIE;
    // 切换到调度器
    SCHEDULER::sched();
    // 不会执行到这里
    assert(0);
    return;
}

std::ostream &operator<<(std::ostream &_os, const TASK::task_t &_task) {
    _os << _task.name << ": ";
    printf("pid 0x%X, state 0x%X\n", _task.pid, _task.state);
    std::cout << _task.context;
    return _os;
}

bool TASK::init(void) {
    // 初始化进程链表
    task_queue = new mystl::queue<task_t *>;
    // 当前进程
    curr_task[CPU::get_curr_core_id()]         = new task_t("init", nullptr);
    curr_task[CPU::get_curr_core_id()]->hartid = CPU::get_curr_core_id();
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task[CPU::get_curr_core_id()]->context);
    task_os = curr_task[CPU::get_curr_core_id()];
    task_queue->pop();
    info("task init.\n");
    return true;
}

bool TASK::init_other_core(void) {
    // 当前进程
    curr_task[CPU::get_curr_core_id()]         = new task_t("init", nullptr);
    curr_task[CPU::get_curr_core_id()]->hartid = CPU::get_curr_core_id();
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task[CPU::get_curr_core_id()]->context);
    task_os = curr_task[CPU::get_curr_core_id()];
    info("task other init.\n");
    return true;
}

pid_t TASK::g_pid = 0;

pid_t TASK::alloc_pid(void) {
    pid_t res = g_pid++;
    return res;
}

void TASK::free_pid(pid_t _pid) {
    _pid = _pid;
    return;
}

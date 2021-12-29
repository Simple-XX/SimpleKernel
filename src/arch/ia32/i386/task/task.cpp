
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

extern "C" void switch_context_init(CPU::context_t *_context);

// 当前任务
TASK::task_t *TASK::curr_task;
// 任务向量
mystl::vector<TASK::task_t *> *TASK::task_queue;
TASK::task_t *                 TASK::task_os;

TASK::task_t::task_t(mystl::string _name, void (*_task)(void)) : name(_name) {
    pid      = alloc_pid();
    state    = RUNNING;
    parent   = nullptr;
    page_dir = VMM::get_pgd();
    // 栈空间分配在内核空间
    stack = PMM::alloc_pages_kernel(4);
    // context.ra             = (uintptr_t)_task;
    // context.callee_regs.sp = stack + COMMON::PAGE_SIZE * 4;
    // context.sscratch       = (uintptr_t)malloc(sizeof(CPU::context_t));
    pt_regs.rbx = (uint32_t)func;
    pt_regs.rdx = (uint32_t)args;
    pt_regs.rip = (uint32_t)kthread_entry;
    // 将新进程添加到链表
    task_queue->push_back(this);
    return;
}

TASK::task_t::~task_t(void) {
    // 回收 pid
    free_pid(pid);
    // 停止子进程
    // 回收页目录
    // 回收栈
    PMM::free_pages(stack, 4);
    // 回收上下文
    // 回收其它
    std::cout << "del task: " << name << std::endl;
    return;
}

void TASK::exit(uint32_t _exit_code) {
    // 从队列中删除
    auto iter = task_queue->begin();
    while (iter != task_queue->end()) {
        if (*iter == curr_task) {
            iter = task_queue->erase(iter);
            delete curr_task;
        }
        else {
            iter++;
        }
    }
    // 切换到调度器
    sched();
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
    task_queue = new mystl::vector<task_t *>;
    // 当前进程
    curr_task = new task_t("init", nullptr);
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task->context);
    task_os = curr_task;
    printf("task init.\n");
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

TASK::task_t *TASK::get_curr_task(void) {
    return curr_task;
}

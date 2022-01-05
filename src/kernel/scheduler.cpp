
/**
 * @file scheduler.h
 * @brief 调度器实现
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

#include "stdint.h"
#include "types.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "scheduler.h"
#include "vmm.h"
#include "cpu.hpp"
#include "core.hpp"

extern "C" void switch_context_init(CPU::context_t *_context);
extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);

task_t                 *SCHEDULER::curr_task[COMMON::CORES_COUNT];
mystl::queue<task_t *> *SCHEDULER::task_queue;
pid_t                   SCHEDULER::g_pid = 1;
task_t                 *SCHEDULER::task_os[COMMON::CORES_COUNT];

pid_t SCHEDULER::alloc_pid(void) {
    pid_t res = g_pid++;
    return res;
}

void SCHEDULER::free_pid(pid_t _pid) {
    _pid = _pid;
    return;
}

// 获取下一个要执行的任务
task_t *SCHEDULER::get_next_task(void) {
    task_t *task = nullptr;
    // TODO: 如果当前任务的本次运行时间超过 1，进行切换
    // 如果任务未结束
    if (curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->state == RUNNING) {
        // 如果不是 os 线程
        if (curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->pid != 0) {
            // 重新加入队列
            task_queue->push(
                curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]);
        }
    }
    // 否则删除
    else {
        rm_task(curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]);
        curr_task[COMMON::get_curr_core_id(CPU::READ_SP())] = nullptr;
    }
    task = task_queue->front();
    task_queue->pop();
    return task;
}

// 切换到下一个任务
void SCHEDULER::switch_task(void) {
    // 获取下一个线程并替换为当前线程下一个线程
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())] = get_next_task();
    // 设置 core 当前线程信息
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].curr_task =
        curr_task[COMMON::get_curr_core_id(CPU::READ_SP())];
    // 切换
    switch_context(
        &task_os[COMMON::get_curr_core_id(CPU::READ_SP())]->context,
        &curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->context);
    return;
}

SCHEDULER &SCHEDULER::get_instance(void) {
    static SCHEDULER scheduler;
    return scheduler;
}

bool SCHEDULER::init(void) {
    // 初始化自旋锁
    spinlock.init("SCHEDULER");
    // 初始化进程队列
    task_queue = new mystl::queue<task_t *>;
    // 当前进程
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())] =
        new task_t("init", nullptr);
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->pid   = 0;
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->state = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(
        &curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->context);
    task_os[COMMON::get_curr_core_id(CPU::READ_SP())] =
        curr_task[COMMON::get_curr_core_id(CPU::READ_SP())];
    // 初始化 core 信息
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].core_id =
        COMMON::get_curr_core_id(CPU::READ_SP());
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].curr_task =
        curr_task[COMMON::get_curr_core_id(CPU::READ_SP())];
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].sched_task =
        task_os[COMMON::get_curr_core_id(CPU::READ_SP())];
    info("task init.\n");
    return true;
}

bool SCHEDULER::init_other_core(void) {
    // 当前进程
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())] =
        new task_t("init other", nullptr);
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->pid   = 0;
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->state = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(
        &curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->context);
    task_os[COMMON::get_curr_core_id(CPU::READ_SP())] =
        curr_task[COMMON::get_curr_core_id(CPU::READ_SP())];
    // 初始化 core 信息
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].core_id =
        COMMON::get_curr_core_id(CPU::READ_SP());
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].curr_task =
        curr_task[COMMON::get_curr_core_id(CPU::READ_SP())];
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].sched_task =
        task_os[COMMON::get_curr_core_id(CPU::READ_SP())];
    info("task other init: 0x%X.\n", COMMON::get_curr_core_id(CPU::READ_SP()));
    return true;
}

void SCHEDULER::sched(void) {
    printf("sched: Running... 0x%X\n",
           COMMON::get_curr_core_id(CPU::READ_SP()));
    // TODO: 根据当前任务的属性进行调度
    switch_task();
    return;
}

task_t *SCHEDULER::get_curr_task(void) {
    spinlock.lock();
    task_t *ret = curr_task[COMMON::get_curr_core_id(CPU::READ_SP())];
    spinlock.unlock();
    return ret;
}

void SCHEDULER::add_task(task_t *_task) {
    spinlock.lock();
    _task->pid   = alloc_pid();
    _task->state = RUNNING;
    // 将新进程添加到链表
    task_queue->push(_task);
    spinlock.unlock();
    return;
}

void SCHEDULER::rm_task(task_t *_task) {
    spinlock.lock();
    // 回收 pid
    free_pid(_task->pid);
    spinlock.unlock();
    return;
}

void SCHEDULER::switch_to_kernel(void) {
    printf("switch_to_kernel 0x%X\n", COMMON::get_curr_core_id(CPU::READ_SP()));
    // 设置 core 当前线程信息
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].curr_task =
        task_os[COMMON::get_curr_core_id(CPU::READ_SP())];
    switch_context(
        &curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->context,
        &task_os[COMMON::get_curr_core_id(CPU::READ_SP())]->context);
    err("switch_to_kernel 0x%X end\n",
        COMMON::get_curr_core_id(CPU::READ_SP()));
    return;
}

void SCHEDULER::exit(uint32_t _exit_code) {
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->exit_code = _exit_code;
    curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->state     = ZOMBIE;
    printf("%s exit: 0x%X\n",
           curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->name.c_str(),
           curr_task[COMMON::get_curr_core_id(CPU::READ_SP())]->exit_code);
    switch_to_kernel();
    assert(0);
    return;
}

extern "C" void exit(int _status) {
    SCHEDULER::get_instance().exit(_status);
}

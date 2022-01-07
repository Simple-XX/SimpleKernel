
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
#include "tmp_scheduler.h"
#include "vmm.h"
#include "cpu.hpp"
#include "core.h"

extern "C" void switch_context_init(CPU::context_t *_context);
extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);

task_t *tmp_SCHEDULER::curr_task[COMMON::CORES_COUNT];

pid_t tmp_SCHEDULER::alloc_pid(void) {
    pid_t res = g_pid++;
    return res;
}

void tmp_SCHEDULER::free_pid(pid_t _pid) {
    _pid = _pid;
    return;
}

// 获取下一个要执行的任务
task_t *tmp_SCHEDULER::get_next_task(void) {
    task_t *task = nullptr;
    // TODO: 如果当前任务的本次运行时间超过 1，进行切换
    // 如果任务未结束
    if (curr_task[COMMON::get_curr_core_id()]->state == RUNNING) {
        // 如果不是 os 线程
        if (curr_task[COMMON::get_curr_core_id()]->pid != 0) {
            // 重新加入队列
            task_queue->push(curr_task[COMMON::get_curr_core_id()]);
        }
    }
    // 否则删除
    else {
        remove_task(curr_task[COMMON::get_curr_core_id()]);
    }
    task = task_queue->front();
    task_queue->pop();
    return task;
}

// 切换到下一个任务
void tmp_SCHEDULER::switch_task(void) {
    // 获取下一个线程并替换为当前线程下一个线程
    curr_task[COMMON::get_curr_core_id()] = get_next_task();
    // 设置 core 当前线程信息
    core_t::set_curr_task(curr_task[COMMON::get_curr_core_id()]);
    // 切换
    switch_context(
        &core_t::cores[COMMON::get_curr_core_id()].sched_task->context,
        &core_t::get_curr_task()->context);
    return;
}

tmp_SCHEDULER::tmp_SCHEDULER(void) : SCHEDULER("tmp scheduler") {
    return;
}

tmp_SCHEDULER::~tmp_SCHEDULER(void) {
    return;
}

tmp_SCHEDULER &tmp_SCHEDULER::get_instance(void) {
    static tmp_SCHEDULER scheduler;
    return scheduler;
}

bool tmp_SCHEDULER::init(void) {
    // 初始化自旋锁
    spinlock.init("tmp_SCHEDULER");
    // 初始化进程队列
    task_queue = new mystl::queue<task_t *>;
    // 当前进程
    curr_task[COMMON::get_curr_core_id()]        = new task_t("init", nullptr);
    curr_task[COMMON::get_curr_core_id()]->pid   = 0;
    curr_task[COMMON::get_curr_core_id()]->state = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task[COMMON::get_curr_core_id()]->context);
    // 初始化 core 信息
    core_t::cores[COMMON::get_curr_core_id()].core_id =
        COMMON::get_curr_core_id();
    core_t::cores[COMMON::get_curr_core_id()].curr_task =
        curr_task[COMMON::get_curr_core_id()];
    core_t::cores[COMMON::get_curr_core_id()].sched_task =
        curr_task[COMMON::get_curr_core_id()];
    info("task init.\n");
    return true;
}

bool tmp_SCHEDULER::init_other_core(void) {
    // 当前进程
    curr_task[COMMON::get_curr_core_id()] = new task_t("init other", nullptr);
    curr_task[COMMON::get_curr_core_id()]->pid   = 0;
    curr_task[COMMON::get_curr_core_id()]->state = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task[COMMON::get_curr_core_id()]->context);
    // 初始化 core 信息
    core_t::cores[COMMON::get_curr_core_id()].core_id =
        COMMON::get_curr_core_id();
    core_t::cores[COMMON::get_curr_core_id()].curr_task =
        curr_task[COMMON::get_curr_core_id()];
    core_t::cores[COMMON::get_curr_core_id()].sched_task =
        curr_task[COMMON::get_curr_core_id()];
    info("task other init: 0x%X.\n", COMMON::get_curr_core_id());
    return true;
}

void tmp_SCHEDULER::sched(void) {
    printf("sched: Running... 0x%X\n", COMMON::get_curr_core_id());
    // TODO: 根据当前任务的属性进行调度
    switch_task();
    return;
}

void tmp_SCHEDULER::add_task(task_t *_task) {
    spinlock.lock();
    _task->pid   = alloc_pid();
    _task->state = RUNNING;
    // 将新进程添加到链表
    task_queue->push(_task);
    spinlock.unlock();
    return;
}

void tmp_SCHEDULER::remove_task(task_t *_task) {
    spinlock.lock();
    // 回收 pid
    free_pid(_task->pid);
    spinlock.unlock();
    return;
}

void tmp_SCHEDULER::switch_to_kernel(void) {
    printf("switch_to_kernel 0x%X\n", COMMON::get_curr_core_id());
    // 设置 core 当前线程信息
    core_t::set_curr_task(core_t::cores[COMMON::get_curr_core_id()].sched_task);
    switch_context(
        &curr_task[COMMON::get_curr_core_id()]->context,
        &core_t::cores[COMMON::get_curr_core_id()].sched_task->context);
    err("switch_to_kernel 0x%X end\n", COMMON::get_curr_core_id());
    return;
}

void tmp_SCHEDULER::exit(uint32_t _exit_code) {
    core_t::get_curr_task()->exit_code = _exit_code;
    core_t::get_curr_task()->state     = ZOMBIE;
    printf("%s exit: 0x%X\n", core_t::get_curr_task()->name.c_str(),
           core_t::get_curr_task()->exit_code);
    switch_to_kernel();
    assert(0);
    return;
}

extern "C" void exit(int _status) {
    tmp_SCHEDULER::get_instance().exit(_status);
}

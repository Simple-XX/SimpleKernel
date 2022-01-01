
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

extern "C" void switch_context_init(CPU::context_t *_context);
extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);

// 当前任务
task_t *SCHEDULER::curr_task[CPU::CPUS];
// 任务向量
mystl::queue<task_t *>  *SCHEDULER::task_queue;
task_t                  *SCHEDULER::task_os;

// 获取下一个要执行的任务
task_t *SCHEDULER::get_next_task(void) {
    task_t *task = nullptr;
    // TODO: 如果当前任务的本次运行时间超过 1，进行切换
    // 如果任务未结束
    if (curr_task[CPU::get_curr_core_id()]->state == RUNNING) {
        // 如果不是 os 线程
        if (curr_task[CPU::get_curr_core_id()]->pid != 0) {
            // 重新加入队列
            task_queue->push(curr_task[CPU::get_curr_core_id()]);
        }
    }
    // 否则删除
    else {
        delete curr_task[CPU::get_curr_core_id()];
        curr_task[CPU::get_curr_core_id()] = nullptr;
    }
    task = task_queue->front();
    task_queue->pop();
    return task;
}

// 切换到下一个任务
void SCHEDULER::switch_task(void) {
    // 获取当前线程
    task_t *prev = curr_task[CPU::get_curr_core_id()];
    // 获取下一个线程并替换为当前线程下一个线程
    curr_task[CPU::get_curr_core_id()] = get_next_task();
    // 切换
    // switch_context(&prev->context,
    //                &curr_task[CPU::get_curr_core_id()]->context);
    switch_context(&task_os->context,
                   &curr_task[CPU::get_curr_core_id()]->context);
    return;
}

void SCHEDULER::sched(void) {
    printf("sched: Running...\n");
    size_t count = 500000000;
    while (count--)
        ;
    // TODO: 根据当前任务的属性进行调度
    switch_task();
    return;
}

void idle(void) {
    while (1) {
        __asm__ volatile("wfi");
    }
    return;
}

bool SCHEDULER::init(void) {
    // 初始化进程队列
    task_queue  = new mystl::queue<task_t *>;
    // 当前进程
    curr_task[CPU::get_curr_core_id()]         = new task_t("init", 0, nullptr);
    curr_task[CPU::get_curr_core_id()]->hartid = CPU::get_curr_core_id();
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task[CPU::get_curr_core_id()]->context);
    task_os = curr_task[CPU::get_curr_core_id()];
    info("task init.\n");
    return true;
}

bool SCHEDULER::init_other_core(void) {
    // 当前进程
    curr_task[CPU::get_curr_core_id()]         = new task_t("init", 0, nullptr);
    curr_task[CPU::get_curr_core_id()]->hartid = CPU::get_curr_core_id();
    // 原地跳转，填充启动进程的 task_t 信息
    switch_context_init(&curr_task[CPU::get_curr_core_id()]->context);
    task_os = curr_task[CPU::get_curr_core_id()];
    info("task other init.\n");
    return true;
}

pid_t SCHEDULER::g_pid = 0;

pid_t SCHEDULER::alloc_pid(void) {
    pid_t res = g_pid++;
    return res;
}

void SCHEDULER::free_pid(pid_t _pid) {
    _pid = _pid;
    return;
}

void SCHEDULER::add_task(task_t *_task) {
    // 将新进程添加到链表
    task_queue->push(_task);
    return;
}

void SCHEDULER::rm_task(task_t *_task) {
    // 回收 pid
    free_pid(_task->pid);
    return;
}

void SCHEDULER::switch_to_kernel(void) {
    printf("switch_to_kernel\n");
    switch_context(&curr_task[CPU::get_curr_core_id()]->context,
                   &task_os->context);

    return;
}

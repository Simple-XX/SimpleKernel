
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// scheduler.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "types.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "scheduler.h"
#include "vmm.h"
#include "cpu.hpp"

extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);

// 获取下一个要执行的任务
TASK::task_t *SCHEDULER::get_next_task(void) {
    TASK::task_t *task = nullptr;
    // TODO: 如果当前任务的本次运行时间超过 1，进行切换
    // 如果任务未结束
    if (TASK::curr_task[CPU::get_curr_core_id()]->state == RUNNING) {
        // 重新加入队列
        TASK::task_queue->push(TASK::curr_task[CPU::get_curr_core_id()]);
    }
    // 否则删除
    else {
        delete TASK::curr_task[CPU::get_curr_core_id()];
        TASK::curr_task[CPU::get_curr_core_id()] = nullptr;
    }
    task = TASK::task_queue->front();
    TASK::task_queue->pop();
    return task;
}

// 切换到下一个任务
void SCHEDULER::switch_task(void) {
    TASK::task_t *prev = TASK::curr_task[CPU::get_curr_core_id()];
    // 下一个线程
    TASK::curr_task[CPU::get_curr_core_id()] = get_next_task();
    // 切换
    switch_context(&prev->context,
                   &TASK::curr_task[CPU::get_curr_core_id()]->context);
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